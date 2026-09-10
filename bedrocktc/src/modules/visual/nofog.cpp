#include "nofog.hpp"
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/hooks/Hooks.hpp>
#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>

namespace bedrocktc {


static void (*_setupFogPlayer_orig)(void* _this, void* screenContext, float a3);
static NoFogModule* g_nofogMod = nullptr;

static void _setupFogPlayer_hook(void* _this, void* screenContext, float a3) {
    if (_setupFogPlayer_orig) {
        _setupFogPlayer_orig(_this, screenContext, a3);
    }

    if (g_nofogMod && g_nofogMod->enabled) {
        uintptr_t base = (uintptr_t)_this;

        
        float* mBaseFogStart = (float*)(base + ::bedrocktc::sdk::offsets::LevelRendererPlayer::mBaseFogStart);
        float* mBaseFogEnd   = (float*)(base + ::bedrocktc::sdk::offsets::LevelRendererPlayer::mBaseFogEnd);
        *mBaseFogStart = 999999.0f;
        *mBaseFogEnd   = 1000000.0f;

        
       uintptr_t camera_base = (uintptr_t)_this;
        float* mCurrentFogDensityMax = (float*)(camera_base + ::bedrocktc::sdk::offsets::LevelRendererPlayer::mCurrentFogDensityMax);
        *mCurrentFogDensityMax = 0.0f;
    }
}

NoFogModule::NoFogModule() : Module("NoFog", "Removes fog to improve visibility.") {
    m_patched = false;
    m_patchTarget = nullptr;
    g_nofogMod = this;
}

NoFogModule::~NoFogModule() {
    if (g_nofogMod == this) g_nofogMod = nullptr;
}

void NoFogModule::onInit() {
    if (m_patchTarget) return;
    uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::SetupFogPlayer); 
    if (addr != 0) {
        m_patchTarget = (void*)addr;
    }
}

void NoFogModule::applyPatch() {
    if (m_patched || !m_patchTarget) return;
    ::bedrocktc::hooks::install(m_patchTarget, (void*)_setupFogPlayer_hook, (void**)&_setupFogPlayer_orig);
    m_patched = true;
}

void NoFogModule::onEnable() {
    applyPatch();
}

void NoFogModule::onDisable() {
}

} // namespace bedrocktc
