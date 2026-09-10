#include <btp/profiles/Profiles.hpp>
#include <btp/hud/Hud.hpp>
#include <btp/gui/Gui.hpp>
#include <btp/modules/ClientModules.hpp>
#include <bedrocktc/BedrockTC.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

namespace btp::profiles {
Profiles& Profiles::get() { static Profiles instance; return instance; }
void Profiles::setDirectory(std::filesystem::path path) { mDirectory = std::move(path); std::error_code ec; std::filesystem::create_directories(mDirectory, ec); }
std::vector<std::string> Profiles::list() const {
    std::vector<std::string> result;
    if (!std::filesystem::exists(mDirectory)) return result;
    for (const auto& entry : std::filesystem::directory_iterator(mDirectory))
        if (entry.is_regular_file() && entry.path().extension() == ".json") result.push_back(entry.path().stem().string());
    return result;
}

bool Profiles::save(const std::string& name) const {
    if (name.empty()) return false;
    nlohmann::json root{{"version", 1}, {"modules", {}}, {"hud", {}}, {"gui", {}}};
    for (auto* module : bedrocktc::ModuleRegistry::get().modules()) {
        if (!module) continue;
        nlohmann::json state;
        module->saveConfig(state);
        root["modules"][module->moduleId] = std::move(state);
    }
    for (const auto& [id, element] : hud::Hud::get().elements()) {
        root["hud"][id] = { {"visible", element.visible}, {"x", element.x}, {"y", element.y}, {"scale", element.scale} };
    }
    for (const auto& module : modules::ClientModules::get().all()) root["btp_modules"][std::string(module.id)] = module.enabled;
    root["gui"]["visible"] = gui::Gui::get().visible();
    root["gui"]["category"] = gui::Gui::get().category();
    root["gui"]["search"] = gui::Gui::get().search();

    std::error_code ec;
    std::filesystem::create_directories(mDirectory, ec);
    std::ofstream out(mDirectory / (name + ".json"), std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << root.dump(2);
    return static_cast<bool>(out);
}

bool Profiles::load(const std::string& name) {
    std::ifstream in(mDirectory / (name + ".json"), std::ios::binary);
    if (!in) return false;
    try {
        nlohmann::json root; in >> root;
        if (root.contains("modules")) {
            for (auto* module : bedrocktc::ModuleRegistry::get().modules()) {
                if (module && root["modules"].contains(module->moduleId)) module->loadConfig(root["modules"][module->moduleId]);
            }
        }
        if (root.contains("hud")) {
            for (auto& [id, state] : root["hud"].items()) {
                auto& element = hud::Hud::get().element(id);
                if (state.contains("visible")) element.visible = state["visible"].get<bool>();
                if (state.contains("x")) element.x = state["x"].get<float>();
                if (state.contains("y")) element.y = state["y"].get<float>();
                if (state.contains("scale")) element.scale = state["scale"].get<float>();
            }
        }
        if (root.contains("btp_modules")) {
            for (const auto& [id, state] : root["btp_modules"].items()) modules::ClientModules::get().setEnabled(id, state.get<bool>());
        }
        if (root.contains("gui")) {
            auto& g = gui::Gui::get();
            if (root["gui"].contains("category")) g.selectCategory(root["gui"]["category"].get<int>());
            if (root["gui"].contains("search")) g.setSearch(root["gui"]["search"].get<std::string>());
        }
        bedrocktc::config::ConfigManager::get().save();
        return true;
    } catch (...) { return false; }
}

bool Profiles::remove(const std::string& name) const {
    std::error_code ec;
    return std::filesystem::remove(mDirectory / (name + ".json"), ec);
}
}
