#pragma once
#include <bedrocktc/memory/Signatures.hpp>
namespace bedrocktc::signatures { using SignatureId=memory::SignatureId; using SignatureDefinition=memory::SignatureDefinition; inline auto resolve(SignatureId id){return memory::resolve(id);} inline auto resolveAll(std::string_view lib="libminecraftpe.so"){return memory::resolveAll(lib);} }
