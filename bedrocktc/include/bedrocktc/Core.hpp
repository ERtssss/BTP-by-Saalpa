#pragma once
#include <filesystem>
#include <string>
#include <bedrocktc/Export.hpp>
namespace bedrocktc::core {
BEDROCKTC_API bool initialize();
BEDROCKTC_API bool initializeIfMinecraftLoaded();
BEDROCKTC_API void shutdown();
BEDROCKTC_API bool minecraftLoaded();
BEDROCKTC_API bool installed();
BEDROCKTC_API bool ready();
BEDROCKTC_API void setConfigPath(const std::filesystem::path& path);
BEDROCKTC_API const std::filesystem::path& resourceDirectory();
BEDROCKTC_API void setResourceDirectory(const std::filesystem::path& path);
BEDROCKTC_API void setOwnerModId(std::string ownerId);
BEDROCKTC_API const std::string& ownerModId();
}
