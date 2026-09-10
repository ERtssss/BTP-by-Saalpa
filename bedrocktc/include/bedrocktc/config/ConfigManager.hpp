#pragma once
#include <pl/Config.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>
namespace bedrocktc::config {
struct ModuleState {
    std::string id;
    bool masterEnabled = false;
    bool keybindActive = true;
    int keybind = 0;
    std::string settingsJson = "{}";
};
struct ConfigData { int version = 1; std::vector<ModuleState> modules; };
class ConfigManager {
public:
    static ConfigManager& get();
    void load(); void save(); void flush();
    std::string getConfigPath() const;
    void setConfigPath(const std::string& path);
private:
    ConfigManager() = default;
    std::filesystem::path mPath;
    ConfigData mData{};
    mutable std::mutex mMutex;
};
}

namespace pl::config {
template <> struct Schema<bedrocktc::config::ConfigData> {
    static constexpr std::string_view title = "Bedrock TC Configuration";
    static constexpr std::string_view description = "Typed Bedrock TC module state and serialized module settings.";
    static constexpr FieldSchema field(std::string_view name) {
        if (name == "version") return {.title="Schema Version", .readOnly=true};
        if (name == "modules") return {.title="Modules"};
        return {};
    }
};
}
