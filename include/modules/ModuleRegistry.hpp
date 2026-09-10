#pragma once
// Compatibility bridge for legacy BTP sources.
#include <bedrocktc/modules/ModuleRegistry.hpp>
namespace bedrocktoolsplus {
using ModuleRegistry = ::bedrocktc::ModuleRegistry;
using Module = ::bedrocktc::Module;
}
