#include <btp/core/ModuleManager.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace btp::core {
ModuleManager& ModuleManager::get() { static ModuleManager instance; return instance; }

bedrocktc::Module* ModuleManager::find(std::string_view id) const {
    return bedrocktc::ModuleRegistry::get().find(id);
}

bool ModuleManager::setEnabled(std::string_view id, bool enabled) {
    auto* module = find(id);
    if (!module) return false;
    module->setMasterEnabled(enabled);
    return true;
}

bool ModuleManager::toggle(std::string_view id) {
    auto* module = find(id);
    if (!module) return false;
    module->toggle();
    return true;
}

Category ModuleManager::categoryOf(const bedrocktc::Module& module) {
    using BC = bedrocktc::ModuleCategory;
    switch (module.category) {
        case BC::Combat: return Category::Combat;
        case BC::Movement: return Category::Movement;
        case BC::Render: return Category::Render;
        case BC::HUD: return Category::HUD;
        case BC::Client: return Category::Settings;
        case BC::Misc: break;
    }
    const std::string id = module.moduleId;
    auto has = [&](std::string_view token) { return id.find(token) != std::string::npos; };
    if (has("Fullbright") || has("MotionBlur") || has("Fog") || has("Color") || has("Zoom") ||
        has("Hitbox") || has("ViewModel") || has("Chunk") || has("Light") || has("Cinematic") ||
        has("Breadcrumb") || has("Shulker") || has("Glass") || has("Skin") || has("Tnt") || has("TNT"))
        return Category::Visuals;
    if (has("AutoSprint") || has("QuickLoot") || has("Nick") || has("Reach")) return Category::Player;
    if (has("WorldTime") || has("WeatherChanger") || has("Compass")) return Category::World;
    if (has("Counter") || has("Keystrokes") || has("Coords") || has("Tablist") || has("SpeedDisplay")) return Category::HUD;
    return Category::Utility;
}

std::vector<ModuleView> ModuleManager::all() const {
    std::vector<ModuleView> out;
    for (auto* module : bedrocktc::ModuleRegistry::get().modules()) {
        if (module) out.push_back({module, categoryOf(*module)});
    }
    return out;
}

static std::string lower(std::string_view value) {
    std::string out(value);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

std::vector<ModuleView> ModuleManager::search(std::string_view query, Category category) const {
    const std::string needle = lower(query);
    std::vector<ModuleView> out;
    for (auto view : all()) {
        if (category != Category::Settings && view.category != category) continue;
        if (needle.empty()) { out.push_back(view); continue; }
        const auto& module = *view.module;
        const std::string name = lower(module.name);
        const std::string id = lower(module.moduleId);
        const std::string description = lower(module.description);
        if (name.find(needle) != std::string::npos || id.find(needle) != std::string::npos || description.find(needle) != std::string::npos)
            out.push_back(view);
    }
    return out;
}

nlohmann::json ModuleManager::settings(std::string_view id) const {
    auto* module = find(id);
    if (!module) return {};
    nlohmann::json result;
    module->saveConfig(result);
    return result;
}

bool ModuleManager::setSetting(std::string_view id, std::string_view key, std::string_view value) {
    auto* module = find(id);
    if (!module) return false;
    if (module->onMenuConfigChanged(key, value)) return true;
    auto json = settings(id);
    try {
        const std::string k(key), v(value);
        if (json.contains(k) && json[k].is_boolean()) json[k] = (v == "true" || v == "1");
        else if (json.contains(k) && json[k].is_number_integer()) json[k] = std::stoi(v);
        else if (json.contains(k) && json[k].is_number_float()) json[k] = std::stof(v);
        else json[k] = v;
        module->loadConfig(json);
        return true;
    } catch (...) { return false; }
}
}
