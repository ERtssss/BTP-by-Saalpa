#include <btp/hud/Hud.hpp>
#include <btp/modules/ClientModules.hpp>
#include <btp/core/Notifications.hpp>
#include <bedrocktc/BedrockTC.hpp>
#include <chrono>
#include <algorithm>
#include <string>

namespace btp::hud {
namespace {
std::chrono::steady_clock::time_point gLastFrame;
double gFrameMs = 0.0;
double gFps = 0.0;
}

Hud& Hud::get() { static Hud instance; return instance; }

void Hud::initialize() {
    if (!mElements.empty()) return;
    constexpr const char* ids[] = {"watermark", "array_list", "custom_crosshair", "notifications", "performance_monitor", "fps", "ping", "coordinates", "cps", "keystrokes", "armor", "potion", "compass", "speed", "combo", "tablist"};
    for (auto id : ids) mElements.emplace(id, Element{std::string(id), true, 10.f, 10.f, 1.f});
    gLastFrame = std::chrono::steady_clock::now();
    mFrameSubscription = bedrocktc::events::bus().subscribe<bedrocktc::events::FrameEvent>([this](auto&) { draw(); });
}

void Hud::shutdown() {
    if (mFrameSubscription) bedrocktc::events::bus().unsubscribe(mFrameSubscription);
    mFrameSubscription = 0;
}

Element& Hud::element(std::string_view id) { return mElements[std::string(id)]; }
const std::unordered_map<std::string, Element>& Hud::elements() const noexcept { return mElements; }
void Hud::reset(std::string_view id) { element(id) = Element{std::string(id), true, 10.f, 10.f, 1.f}; }

void Hud::draw() {
    const auto now = std::chrono::steady_clock::now();
    gFrameMs = std::chrono::duration<double, std::milli>(now - gLastFrame).count();
    gLastFrame = now;
    if (gFrameMs > 0.0) gFps = 1000.0 / gFrameMs;

    auto& client = modules::ClientModules::get();
    core::Notifications::get().update(static_cast<float>(std::max(0.0, gFrameMs) / 1000.0));

    const auto drawAt = [this](std::string_view id) {
        const auto it = mElements.find(std::string(id));
        return it != mElements.end() && it->second.visible;
    };

    if (client.watermark().enabled && drawAt("watermark"))
        bedrocktc::render::text("btp.watermark", 10, 10, 200, 24, 18, "BTP by Saalpa", 0xFFFFFFFF);

    if (client.arrayList().enabled && drawAt("array_list")) {
        float y = 42.f;
        for (auto* module : bedrocktc::ModuleRegistry::get().modules()) {
            if (module && module->enabled && module->showInMenu) {
                bedrocktc::render::text("btp.array_list", 10, y, 300, 20, 13, module->name, 0xFFFFFFFF);
                y += 18.f;
            }
        }
    }

    if (client.performanceMonitor().enabled && drawAt("performance_monitor")) {
        const std::string text = "FPS: " + std::to_string(static_cast<int>(gFps)) + "  Frame: " + std::to_string(static_cast<int>(gFrameMs)) + " ms";
        bedrocktc::render::text("btp.performance_monitor", 10, 700, 360, 24, 13, text, 0xFFFFFFFF);
    }

    if (client.notifications().enabled && drawAt("notifications")) core::Notifications::get().draw();
    if (client.customCrosshair().enabled && drawAt("custom_crosshair")) {
        bedrocktc::render::line("btp.custom_crosshair", 620, 360, 20, 0, 0xFFFFFFFF);
        bedrocktc::render::line("btp.custom_crosshair", 630, 350, 0, 20, 0xFFFFFFFF);
    }
}
}
