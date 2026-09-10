#pragma once
#include <bedrocktc/version/Signatures.hpp>
#include <bedrocktc/version/offsets/Core.hpp>
#include <bedrocktc/version/offsets/World.hpp>
#include <bedrocktc/version/offsets/Render.hpp>
#include <bedrocktc/version/offsets/Network.hpp>
#include <bedrocktc/version/offsets/Skin.hpp>
#include <bedrocktc/version/offsets/UI.hpp>
#include <bedrocktc/version/offsets/Inventory.hpp>
namespace bedrocktc::version {
struct VersionData {
    std::string_view minecraftLibrary;
    std::string_view sourceSnapshot;
};
inline constexpr VersionData currentData{ "libminecraftpe.so", "Bedrock TC" };
}
