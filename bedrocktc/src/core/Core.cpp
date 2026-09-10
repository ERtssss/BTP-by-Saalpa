#include <bedrocktc/Core.hpp>
#include <bedrocktc/events/EventBus.hpp>
#include <bedrocktc/events/Events.hpp>
#include <bedrocktc/config/ConfigManager.hpp>
#include <bedrocktc/modules/ModuleRegistry.hpp>
#include <bedrocktc/render/Render.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/hooks/Hooks.hpp>
#include "hooks/GameHooks.hpp"
#include <pl/Input.hpp>
#include <dlfcn.h>
#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <exception>
#include <btp/debug/Log.hpp>

namespace bedrocktc::core {
namespace {
std::atomic_bool gLoaded{false};
std::atomic_bool gInitialized{false};
std::mutex gMutex;
std::filesystem::path gResourceDirectory;
std::string gOwnerModId = "btp";
bool gEventsWired = false;
int gContainerDepth = 0, gChatDepth = 0;
void* (*gDlopenOriginal)(const char*, int) = nullptr;
bedrocktc::hooks::Handle gDlopenHook = nullptr;
thread_local bool gInDlopenDetour = false;

void* dlopenDetour(const char* filename, int flags) {
    void* handle = gDlopenOriginal ? gDlopenOriginal(filename, flags) : nullptr;
    if (handle && filename && std::strstr(filename, "libminecraftpe.so") && !gInDlopenDetour) {
        gInDlopenDetour = true;
        try {
            initialize();
        } catch (const std::exception& e) {
            BTP_LOGE("dlopenDetour initialization exception: %s", e.what());
        } catch (...) {
            BTP_LOGE("dlopenDetour initialization unknown exception");
        }
        gInDlopenDetour = false;
    }
    return handle;
}

bool installDlopenWatcher() {
    if (gDlopenHook) return true;
    auto lib = hooks::openLibrary("libdl.so");
    if (!lib) return false;
    auto symbol = reinterpret_cast<void*>(hooks::symbol(lib, "dlopen"));
    if (symbol) gDlopenHook = hooks::install(symbol, reinterpret_cast<void*>(dlopenDetour), reinterpret_cast<void**>(&gDlopenOriginal));
    hooks::closeLibrary(lib);
    return gDlopenHook != nullptr;
}

bool touchCallback(const pl::input::TouchEvent& input) {
    events::TouchInputEvent event{input.action, input.pointerId, input.x, input.y};
    events::bus().publish(event);
    return event.cancelled();
}

bool keyCallback(const pl::input::KeyEvent& input) {
    events::KeyInputEvent event{input.keyCode, input.unicodeChar, input.isKeyDown};
    events::bus().publish(event);
    return event.cancelled();
}

bool mouseCallback(const pl::input::MouseEvent& input) {
    events::MouseInputEvent event{input.button, input.isDown};
    events::bus().publish(event);
    return event.cancelled();
}

void wireEvents() {
    if (gEventsWired) return;
    gEventsWired = true;
    events::bus().subscribe<events::FrameEvent>([](auto&) { ModuleRegistry::get().onFrame(); });
    events::bus().subscribe<events::MouseInputEvent>([](auto& e) {
        if (ModuleRegistry::get().onMouseEvent(e.button, e.down)) e.cancel();
    });
    events::bus().subscribe<events::ScreenStateEvent>([](auto& e) {
        int& depth = e.screen == events::ScreenKind::Container ? gContainerDepth : gChatDepth;
        if (e.phase == events::ScreenPhase::Opened) ++depth;
        else if (depth > 0) --depth;
        ModuleRegistry::get().setKeybindBlocked(gContainerDepth > 0 || gChatDepth > 0);
    });
    pl::input::registerTouchCallback(touchCallback);
    pl::input::registerKeyCallback(keyCallback);
    pl::input::registerMouseCallback(mouseCallback);
}
}

bool initialize() {
    std::lock_guard lock(gMutex);
    if (gInitialized.load()) return true;

    try {
        void* lib = dlopen("libminecraftpe.so", RTLD_NOW | RTLD_NOLOAD);
        if (!lib) {
            gLoaded = false;
            installDlopenWatcher();
            return false;
        }
        dlclose(lib);
        gLoaded = true;

        BTP_LOGI("BedrockTC initialize: resolving signatures");
        if (!memory::resolveAll("libminecraftpe.so")) {
            BTP_LOGE("BedrockTC initialize: signature resolution failed");
            return false;
        }

        BTP_LOGI("BedrockTC initialize: installing game hooks");
        if (!gamehooks::install()) {
            BTP_LOGE("BedrockTC initialize: game hook installation failed");
            return false;
        }

        // BT-style lifecycle, but BTP owns an actual in-game overlay.
        // NativeGui is only the bridge to Minecraft's UI render context;
        // the BTP menu itself is drawn by btp::gui::Gui.
        BTP_LOGI("BedrockTC initialize: installing in-game GUI render hooks");
        if (!render::native::initialize()) {
            BTP_LOGE("BedrockTC initialize: in-game GUI render hook installation failed");
            return false;
        }
        BTP_LOGI("BedrockTC initialize: registering modules");
        registerAllModules();

        BTP_LOGI("BedrockTC initialize: wiring events");
        wireEvents();

        BTP_LOGI("BedrockTC initialize: initializing modules");
        ModuleRegistry::get().initialize();

        BTP_LOGI("BedrockTC initialize: loading config");
        try {
            config::ConfigManager::get().load();
            BTP_LOGI("BedrockTC initialize: config loaded");
        } catch (const std::exception& e) {
            BTP_LOGE("BedrockTC initialize: config load failed: %s", e.what());
        } catch (...) {
            BTP_LOGE("BedrockTC initialize: config load failed: unknown exception");
        }

        // BTP owns its in-game menu. Do not register the module list with
        // LeviLauncher's ModMenu; that would create a second, launcher-side UI.
        BTP_LOGI("BedrockTC initialize: LeviLauncher ModMenu registration disabled; BTP Menu is active");

        gInitialized = true;
        BTP_LOGI("BedrockTC initialize complete");
        return true;
    } catch (const std::exception& e) {
        BTP_LOGE("BedrockTC initialize exception: %s", e.what());
        return false;
    } catch (...) {
        BTP_LOGE("BedrockTC initialize unknown exception");
        return false;
    }
}

bool initializeIfMinecraftLoaded() { return initialize(); }

void shutdown() {
    std::lock_guard lock(gMutex);
    config::ConfigManager::get().flush();
    ModuleRegistry::get().shutdown();
    gamehooks::uninstall();
    hooks::removeAll();
    events::bus().clear();
    if (gDlopenHook) { hooks::remove(gDlopenHook); gDlopenHook = nullptr; gDlopenOriginal = nullptr; }
    gInitialized = false;
}

bool minecraftLoaded() { return gLoaded.load(); }
bool installed() { return gInitialized.load(); }
bool ready() { return gInitialized.load(); }
void setConfigPath(const std::filesystem::path& p) { config::ConfigManager::get().setConfigPath(p.string()); }
const std::filesystem::path& resourceDirectory() { return gResourceDirectory; }
void setResourceDirectory(const std::filesystem::path& p) { gResourceDirectory = p; }
void setOwnerModId(std::string ownerId) { if (!ownerId.empty()) gOwnerModId = std::move(ownerId); }
const std::string& ownerModId() { return gOwnerModId; }
}
