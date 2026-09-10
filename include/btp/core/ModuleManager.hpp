#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <bedrocktc/modules/ModuleRegistry.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace btp::core {
enum class Category { Combat, Visuals, Player, Movement, World, HUD, Render, Utility, Misc, Settings };
struct ModuleView { bedrocktc::Module* module{}; Category category{Category::Misc}; };

class ModuleManager {
public:
    static ModuleManager& get();
    bedrocktc::Module* find(std::string_view id) const;
    bool setEnabled(std::string_view id, bool enabled);
    bool toggle(std::string_view id);
    std::vector<ModuleView> all() const;
    std::vector<ModuleView> search(std::string_view query, Category category) const;
    nlohmann::json settings(std::string_view id) const;
    bool setSetting(std::string_view id, std::string_view key, std::string_view value);
    static Category categoryOf(const bedrocktc::Module& module);
private:
    ModuleManager() = default;
};
}
