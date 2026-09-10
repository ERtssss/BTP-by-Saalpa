#include "ConfigManager.hpp"
#include <bedrocktc/modules/ModuleRegistry.hpp>
#include <pl/Config.hpp>
namespace bedrocktc::config {
ConfigManager& ConfigManager::get() { static ConfigManager instance; return instance; }
std::string ConfigManager::getConfigPath() const { std::lock_guard lock(mMutex); return mPath.empty() ? std::string{} : mPath.string(); }
void ConfigManager::setConfigPath(const std::string& path) { std::lock_guard lock(mMutex); mPath = path; }
void ConfigManager::load() {
    std::lock_guard lock(mMutex);
    if (mPath.empty()) return;
    pl::config::ConfigFile<ConfigData> file(mData, mPath, mPath.parent_path() / "config.schema.json");
    if (!file.load()) return;
    mData = file.value();
    for (auto* mod : ModuleRegistry::get().modules()) {
        if (!mod) continue;
        for (const auto& state : mData.modules) {
            if (state.id != mod->moduleId) continue;
            try {
                nlohmann::json j = nlohmann::json::parse(state.settingsJson);
                mod->loadConfig(j);
            } catch (...) {}
            break;
        }
    }
}
void ConfigManager::save() {
    std::lock_guard lock(mMutex);
    mData.modules.clear();
    for (auto* mod : ModuleRegistry::get().modules()) {
        if (!mod) continue;
        nlohmann::json j; mod->saveConfig(j);
        mData.modules.push_back(ModuleState{mod->moduleId, mod->masterEnabled, mod->keybindActive, mod->keybind, j.dump()});
    }
    if (mPath.empty()) return;
    pl::config::ConfigFile<ConfigData> file(mData, mPath, mPath.parent_path() / "config.schema.json");
    file.save(); file.writeSchema();
}
void ConfigManager::flush() { save(); }
}
