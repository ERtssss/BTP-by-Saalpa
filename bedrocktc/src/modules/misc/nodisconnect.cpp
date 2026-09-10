#include "nodisconnect.hpp"
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/hooks/Hooks.hpp>

namespace bedrocktc {


static bool (*original_isInEDUMultiplayerSession)(void* _this) = nullptr;
static NoDisconnectModule* g_noDisconnectMod = nullptr;

static bool isInEDUMultiplayerSession_hook(void* _this) {
    if (g_noDisconnectMod && g_noDisconnectMod->enabled) {
        return true;
    }
    
    if (original_isInEDUMultiplayerSession) {
        return original_isInEDUMultiplayerSession(_this);
    }
    return false;
}

NoDisconnectModule::NoDisconnectModule()
    : Module("No Disconnect", "Prevents you from being disconnected when you minimize the app.") {
    g_noDisconnectMod = this;
}

void NoDisconnectModule::onInit() {
    if (m_patchTarget) return;
    
    uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::EduMultiplayer);
    if (addr != 0) {
        m_patchTarget = (void*)addr;
        ::bedrocktc::hooks::install(m_patchTarget, (void*)isInEDUMultiplayerSession_hook, (void**)&original_isInEDUMultiplayerSession);
        m_patched = true;
    }
}

void NoDisconnectModule::onEnable() {
}

void NoDisconnectModule::onDisable() {
}


} // namespace bedrocktc
