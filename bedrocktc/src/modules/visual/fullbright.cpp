#include "fullbright.hpp"
#include <bedrocktc/memory/Signatures.hpp>
#include <cstring>
#include <span>

namespace bedrocktc {


static constexpr size_t FULLBRIGHT_PATCH_SIZE = 12;

FullbrightModule::FullbrightModule()
    : Module("Fullbright", "Removes darkness by setting the game's light level to maximum everywhere") {}

void FullbrightModule::onInit() {
    if (m_patchTarget) return;
    uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::Fullbright);
    if (addr == 0) return;
    m_patchTarget = reinterpret_cast<void*>(addr);
}

void FullbrightModule::onEnable() {
    if (m_patched || !m_patchTarget) return;
    
    uint8_t patch[FULLBRIGHT_PATCH_SIZE] = {
        0x40, 0x8F, 0xA8, 0x52,
        0x00, 0x00, 0x27, 0x1E,
        0xC0, 0x03, 0x5F, 0xD6
    };

    m_patchHandle = ::bedrocktc::patches::apply(reinterpret_cast<std::uintptr_t>(m_patchTarget), std::span<const std::uint8_t>(patch, FULLBRIGHT_PATCH_SIZE), "bedrocktc.fullbright");
    m_patched = static_cast<bool>(m_patchHandle);
}

void FullbrightModule::onDisable() {
    if (!m_patched) return;
    ::bedrocktc::patches::remove(m_patchHandle);
    m_patched = false;
}


} // namespace bedrocktc
