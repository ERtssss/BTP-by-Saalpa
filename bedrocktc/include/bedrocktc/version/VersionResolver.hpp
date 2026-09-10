#pragma once
#include <bedrocktc/version/VersionData.hpp>
namespace bedrocktc::version {
inline const VersionData& current() noexcept { return currentData; }
}
