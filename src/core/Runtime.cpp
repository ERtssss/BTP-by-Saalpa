#include <btp/core/Runtime.hpp>
#include <btp/config/Config.hpp>
#include <btp/profiles/Profiles.hpp>
#include <btp/hud/Hud.hpp>
#include <btp/modules/ClientModules.hpp>
#include <btp/core/Notifications.hpp>
#include <bedrocktc/BedrockTC.hpp>
#include <system_error>
#include <btp/debug/Log.hpp>
#include <dlfcn.h>
#include <btp/gui/Gui.hpp>

namespace btp::core {
Runtime& Runtime::get() { static Runtime instance; return instance; }

bool Runtime::load(ll::mod::NativeMod& self) {
    BTP_LOGI("Runtime::load begin");
    if (mActive.load()) { BTP_LOGI("Runtime::load already active"); return true; }

    mOwnerModId = self.getId();
    self.getLogger().info("BTP {} by {} v{}; entry={} library={} manifest={} icon={} modDir={} dataDir={} configDir={} resources={} JavaVM={} state={}",
        self.getName(), self.getAuthor(), self.getVersion(), self.getEntryFileName(),
        self.getLibraryPath().string(), self.getManifestPath().string(), self.getIconPath().string(),
        self.getModDir().string(), self.getDataDir().string(), self.getConfigDir().string(),
        self.getResourceDir().string(), static_cast<const void*>(self.getJavaVM()), static_cast<int>(self.getState()));

    mConfigDir = self.getConfigDir();
    if (mConfigDir.empty()) {
        BTP_LOGE("Runtime::load config directory is empty");
        return false;
    }

    btp::debug::setLogFile(mConfigDir / "btp-debug.log");
    BTP_LOGI("Debug log file: %s", (mConfigDir / "btp-debug.log").string().c_str());

    std::error_code ec;
    std::filesystem::create_directories(mConfigDir / "profiles", ec);
    if (ec) {
        BTP_LOGE("Runtime::load create_directories failed: %s", ec.message().c_str());
        return false;
    }

    config::Config::get().setDirectory(mConfigDir);
    bedrocktc::core::setOwnerModId(mOwnerModId);
    bedrocktc::core::setConfigPath(mConfigDir / "bedrocktc.json");
    bedrocktc::core::setResourceDirectory(self.getResourceDir());
    profiles::Profiles::get().setDirectory(mConfigDir / "profiles");

    BTP_LOGI("Runtime::load complete: configDir=%s", mConfigDir.string().c_str());
    return true;
}

bool Runtime::enable() {
    BTP_LOGI("Runtime::enable begin");
    if (mActive.exchange(true)) {
        BTP_LOGI("Runtime::enable already active");
        return true;
    }

    // Same lifecycle as the working BedrockTools runtime:
    // if Minecraft is already loaded, initialize immediately; otherwise
    // BedrockTC installs its dlopen watcher and initializes when Minecraft
    // loads libminecraftpe.so.
    BTP_LOGI("Runtime::enable calling BedrockTC initializeIfMinecraftLoaded");
    const bool initialized = bedrocktc::core::initializeIfMinecraftLoaded();
    BTP_LOGI("Runtime::enable BedrockTC returned %s; ready=%s",
             initialized ? "true" : "false",
             bedrocktc::core::ready() ? "YES" : "NO");

    auto finishPostInit = [this]() {
        if (mPostInitDone || !bedrocktc::core::ready()) return;
        mPostInitDone = true;
        hud::Hud::get().initialize();
        BTP_LOGI("Runtime::postInit HUD initialized");
        if (!config::Config::get().load()) {
            BTP_LOGW("Runtime::postInit BTP config load failed");
        }
        btp::gui::Gui::get().initialize();
        BTP_LOGI("Runtime::postInit in-game GUI initialized");
        if (mFrameSubscription) {
            bedrocktc::events::bus().unsubscribe(mFrameSubscription);
            mFrameSubscription = 0;
        }
        Notifications::get().push(NotificationType::Success, "BTP enabled");
    };

    // A frame callback lets the same code work when Minecraft loads after
    // Runtime::enable(). BedrockTC wires the event bus during its BT-style
    // initialization, so the callback becomes active on the next frame.
    mFrameSubscription = bedrocktc::events::bus().subscribe<bedrocktc::events::FrameEvent>(
        [finishPostInit](auto&) mutable { finishPostInit(); },
        bedrocktc::events::EventPriority::Last);

    if (initialized) finishPostInit();
    else BTP_LOGI("Runtime::enable Minecraft is not ready yet; dlopen watcher will finish initialization");

    BTP_LOGI("Runtime::enable complete");
    return true;
}

bool Runtime::disable() {
    BTP_LOGI("Runtime::disable begin");
    if (!mActive.exchange(false)) return true;

    config::Config::get().save();
    btp::gui::Gui::get().shutdown();
    if (mFrameSubscription) {
        bedrocktc::events::bus().unsubscribe(mFrameSubscription);
        mFrameSubscription = 0;
    }
    mPostInitDone = false;
    hud::Hud::get().shutdown();
    modules::ClientModules::get().shutdown();
    bedrocktc::core::shutdown();
    BTP_LOGI("Runtime::disable complete");
    return true;
}

bool Runtime::unload() { return disable(); }
bool Runtime::active() const noexcept { return mActive.load(); }
const std::filesystem::path& Runtime::configDir() const noexcept { return mConfigDir; }
}
