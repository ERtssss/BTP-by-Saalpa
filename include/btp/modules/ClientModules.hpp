#pragma once
#include <array>
#include <string_view>

namespace btp::modules {
struct Module {
    std::string_view id;
    std::string_view name;
    std::string_view description;
    bool enabled = false;
};
class ClientModules {
public:
    static ClientModules& get();
    void initialize();
    void shutdown();
    Module& watermark();
    Module& arrayList();
    Module& customCrosshair();
    Module& notifications();
    Module& performanceMonitor();
    const std::array<Module, 5>& all() const noexcept;
    Module* find(std::string_view id) noexcept;
    bool setEnabled(std::string_view id, bool enabled) noexcept;
private:
    std::array<Module, 5> mModules{{
        {"btp.watermark", "Watermark", "BTP watermark", true},
        {"btp.array_list", "Array List", "List enabled Bedrock TC modules", true},
        {"btp.custom_crosshair", "Custom Crosshair", "Client-side crosshair overlay", false},
        {"btp.notifications", "Notifications", "BTP notification overlay", true},
        {"btp.performance_monitor", "Performance Monitor", "FPS and frame-time monitor", true}
    }};
};
}
