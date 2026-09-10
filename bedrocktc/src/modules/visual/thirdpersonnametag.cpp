#include "thirdpersonnametag.hpp"
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/Offsets.hpp>

namespace bedrocktc {


ThirdPersonNametagModule::ThirdPersonNametagModule() : Module("Third Person Nametag", "Shows your own nametag in third person view.") {
    m_patched = false;
    m_patchTarget = nullptr;
}

ThirdPersonNametagModule::~ThirdPersonNametagModule() {
    removePatch();
}

void ThirdPersonNametagModule::onInit() {
    if (m_patchTarget) return;
    uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::Nametag);
    if (addr != 0) {
        
        m_patchTarget = (void*)(addr + ::bedrocktc::sdk::offsets::NameTag::mExtractNameTagsPatchOffset);

    }
}

void ThirdPersonNametagModule::applyPatch() {
    if (m_patched || !m_patchTarget) return;
    uint32_t nop = 0xD503201F;
    m_patchHandle = ::bedrocktc::patches::apply(reinterpret_cast<std::uintptr_t>(m_patchTarget), nop, "bedrocktc.thirdperson_nametag");
    m_patched = static_cast<bool>(m_patchHandle);
}

void ThirdPersonNametagModule::removePatch() {
    if (!m_patched) return;
    ::bedrocktc::patches::remove(m_patchHandle);
    m_patched = false;
}

void ThirdPersonNametagModule::onEnable() {
    applyPatch();
}

void ThirdPersonNametagModule::onDisable() {
    removePatch();
}


} // namespace bedrocktc
