#include <btp/config/Config.hpp>
#include <utility>

namespace btp::config {
Config& Config::get() { static Config instance; return instance; }

void Config::setDirectory(std::filesystem::path path) {
    mDirectory = std::move(path);
    mFile = pl::config::ConfigFile<ConfigData>(ConfigData{}, file(), schemaFile());
}

const std::filesystem::path& Config::directory() const noexcept { return mDirectory; }
std::filesystem::path Config::file() const { return mDirectory / "config.json"; }
std::filesystem::path Config::schemaFile() const { return mDirectory / "config.schema.json"; }
bool Config::load() { return mFile.load(); }
bool Config::save() const { return mFile.save() && mFile.writeSchema(); }
}
