#pragma once
#include <bedrocktc/version/Signatures.hpp>
namespace bedrocktc::memory {
using SignatureId = version::SignatureId;
using SignatureDefinition = version::SignatureDefinition;
inline constexpr auto SignatureCount = version::SignatureCount;
inline bool resolveAll(std::string_view libraryName="libminecraftpe.so"){return version::resolveAll(libraryName);}
inline std::uintptr_t resolve(SignatureId id){return version::resolve(id);}
inline void clear(){version::clear();}
}
