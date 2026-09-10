#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <pl/Config.hpp>

namespace btp::config {

enum class Theme { Dark, Light, System };

struct UiConfig {
    bool visible = false;
    int category = 0;
    std::string search{};
    Theme theme = Theme::Dark;
    std::optional<std::string> selectedModule{};
    std::vector<std::string> recentProfiles{};
};

struct ConfigData {
    int version = 1;
    bool enabled = true;
    UiConfig ui{};
};

class Config {
public:
    static Config& get();
    void setDirectory(std::filesystem::path path);
    const std::filesystem::path& directory() const noexcept;
    std::filesystem::path file() const;
    std::filesystem::path schemaFile() const;
    bool load();
    bool save() const;
    ConfigData& value() noexcept { return mFile.value(); }
    const ConfigData& value() const noexcept { return mFile.value(); }
private:
    Config() = default;
    std::filesystem::path mDirectory;
    pl::config::ConfigFile<ConfigData> mFile{ConfigData{}, {}, {}};
};
}

namespace pl::config {
template <> struct Schema<btp::config::ConfigData> {
    static constexpr std::string_view title = "BTP Configuration";
    static constexpr std::string_view description = "Typed BTP runtime and UI configuration.";
    static constexpr FieldSchema field(std::string_view name) {
        if (name == "version") return {.title="Schema Version", .description="Configuration schema version", .readOnly=true};
        if (name == "enabled") return {.title="Enabled", .description="Enable BTP at startup"};
        if (name == "ui") return {.title="UI", .description="BTP user interface state"};
        return {};
    }
};
template <> struct Schema<btp::config::UiConfig> {
    static constexpr std::string_view title = "UI";
    static constexpr std::string_view description = "BTP interface preferences.";
    static constexpr FieldSchema field(std::string_view name) {
        if (name == "category") return {.title="Category", .description="Selected module category", .minimum=0, .maximum=9};
        return {};
    }
};
}
