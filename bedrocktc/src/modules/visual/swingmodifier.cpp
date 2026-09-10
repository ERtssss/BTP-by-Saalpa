#include "swingmodifier.hpp"
#include <bedrocktc/hooks/Hooks.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/Offsets.hpp>

namespace bedrocktc {


static SwingModifierModule* g_swingMod = nullptr;

SwingModifierModule::SwingModifierModule() : Module("Swing Modifier", "Modify ur swing like flux swing and the speed(visually)") {
    m_patched = false;
    m_renderFirstPersonHooked = false;
    m_getModifiedSwingDurationHooked = false;
    m_patchTarget = nullptr;
    m_patchTarget2 = nullptr;
    g_swingMod = this;
}

SwingModifierModule::~SwingModifierModule() {
    if (g_swingMod == this) g_swingMod = nullptr;
}

static void (*_renderFirstPerson_orig)(void*, void*, const void*, std::uint8_t) = nullptr;
static void _renderFirstPerson_hook(void* self, void* renderContext, const void* prevProj, std::uint8_t itemFlags) {
    if(g_swingMod) {
        if(g_swingMod->m_fluxSwing && g_swingMod->enabled) g_swingMod->applyPatch();
        else g_swingMod->removePatch();
    }
    _renderFirstPerson_orig(self, renderContext, prevProj, itemFlags);
}

static int (*_getModifiedSwingDuration_orig)(void*) = nullptr;
static int _getModifiedSwingDuration_hook(void* self) {
    if(!g_swingMod || !g_swingMod->enabled) return _getModifiedSwingDuration_orig(self);
    return g_swingMod->m_swingSpeed;
}

void SwingModifierModule::onInit() {
    uintptr_t renderFirstPerson = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::ItemInHandRendererRenderFirstPerson);
    if (renderFirstPerson != 0) {
        if(!m_patchTarget){
            m_patchTarget = (void*)(renderFirstPerson + ::bedrocktc::sdk::offsets::ItemInHandRenderer::mRenderFirstPersonTransformPatchOffset1);
            m_patchTarget2 = (void*)(renderFirstPerson + ::bedrocktc::sdk::offsets::ItemInHandRenderer::mRenderFirstPersonTransformPatchOffset2);

        }
        if(!m_renderFirstPersonHooked){
            ::bedrocktc::hooks::install((void*)renderFirstPerson, (void*)_renderFirstPerson_hook, (void**)&_renderFirstPerson_orig);
            m_renderFirstPersonHooked = true;
        }
    }
    if(!m_getModifiedSwingDurationHooked){
        uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::MobGetModifiedSwingDuration);
        if (addr != 0) {
            ::bedrocktc::hooks::install((void*)addr, (void*)_getModifiedSwingDuration_hook, (void**)&_getModifiedSwingDuration_orig);
            m_getModifiedSwingDurationHooked = true;
        }
    }
}

void SwingModifierModule::applyPatch() {
    if (m_patched || !m_patchTarget) return;
    uint32_t nop = 0xD503201F;
    m_patchHandle1 = ::bedrocktc::patches::apply(reinterpret_cast<std::uintptr_t>(m_patchTarget), nop, "bedrocktc.swing_modifier.1");
    m_patchHandle2 = ::bedrocktc::patches::apply(reinterpret_cast<std::uintptr_t>(m_patchTarget2), nop, "bedrocktc.swing_modifier.2");
    m_patched = static_cast<bool>(m_patchHandle1) && static_cast<bool>(m_patchHandle2);
}

void SwingModifierModule::removePatch() {
    if (!m_patched) return;
    ::bedrocktc::patches::remove(m_patchHandle1);
    ::bedrocktc::patches::remove(m_patchHandle2);
    m_patched = false;
}

void SwingModifierModule::onEnable() {
}

void SwingModifierModule::onDisable() {
    removePatch();
}

void SwingModifierModule::loadConfig(const nlohmann::json& j) {
    Module::loadConfig(j);
    if (j.contains("m_fluxSwing")) m_fluxSwing = j["m_fluxSwing"].get<bool>();
    if (j.contains("m_swingSpeed")) m_swingSpeed = j["m_swingSpeed"].get<int>();
}

void SwingModifierModule::saveConfig(nlohmann::json& j) {
    Module::saveConfig(j);
    j["m_fluxSwing"] = m_fluxSwing;
    j["m_swingSpeed"] = m_swingSpeed;
}

} // namespace bedrocktc
