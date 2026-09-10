#include "zoom.hpp"
#include <bedrocktc/hooks/Hooks.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/Memory.hpp>
#include <cmath>
#include <algorithm>

namespace bedrocktc {


static ZoomModule* g_zoomMod = nullptr;

static float (*_getFov_orig)(void*, float, int) = nullptr;

static float _getFov_zoom_hook(void* _this, float a, int enableVariableFOV) {
    float originalFov = 0.0f;
    if (_getFov_orig)
        originalFov = _getFov_orig(_this, a, enableVariableFOV);
    
    if (!g_zoomMod) return originalFov;

    if (originalFov == 70.0f || originalFov == 60.0f) {
        return originalFov;
    }

    g_zoomMod->m_baseFov = originalFov;

    if (g_zoomMod->m_isFirstTime) {
        g_zoomMod->m_currentFov = originalFov;
        g_zoomMod->m_isFirstTime = false;
    }

    if (g_zoomMod->isZoomActive()) {
        g_zoomMod->m_animationFinished = false;
        g_zoomMod->m_currentFov = std::lerp(g_zoomMod->m_currentFov, g_zoomMod->m_targetZoomFov, g_zoomMod->m_animSpeed);
        return g_zoomMod->m_currentFov;
    } else {
        if (!g_zoomMod->m_animationFinished) {
            g_zoomMod->m_currentFov = std::lerp(g_zoomMod->m_currentFov, originalFov, g_zoomMod->m_animSpeed);
            if (std::abs(g_zoomMod->m_currentFov - originalFov) < 0.5f) {
                g_zoomMod->m_animationFinished = true;
                g_zoomMod->m_currentFov = originalFov;
            }
            return g_zoomMod->m_currentFov;
        }
    }

    return originalFov;
}


struct Vec2 { float x, y; };
static void (*_applyTurnDelta_orig)(void*, Vec2*) = nullptr;

static void _applyTurnDelta_hook(void* _this, Vec2* rotationDelta) {
    if (g_zoomMod && (g_zoomMod->isZoomActive() || !g_zoomMod->m_animationFinished) && g_zoomMod->m_lowSens && g_zoomMod->m_baseFov > 0.1f) {
        float zoomRatio = g_zoomMod->m_currentFov / g_zoomMod->m_baseFov;
        float strength = g_zoomMod->m_lowSensStrength;
        float multiplier = 1.0f - (1.0f - zoomRatio) * strength;
        multiplier = std::max(0.01f, std::min(1.0f, multiplier));
        
        Vec2 modifiedDelta = { rotationDelta->x * multiplier, rotationDelta->y * multiplier };
        if (_applyTurnDelta_orig)
            _applyTurnDelta_orig(_this, &modifiedDelta);
    } else {
        if (_applyTurnDelta_orig)
            _applyTurnDelta_orig(_this, rotationDelta);
    }
}


static bool (*_getHideItemInHand_orig)(void*) = nullptr;

static bool _getHideItemInHand_hook(void* _this) {
    bool hide = false;
    if (_getHideItemInHand_orig)
        hide = _getHideItemInHand_orig(_this);
    
    if (g_zoomMod && g_zoomMod->isZoomActive() && g_zoomMod->m_hideHand) {
        return true;
    }
    
    return hide;
}

ZoomModule::ZoomModule() 
    : Module("Zoom", "Smoothly zooms your camera like OptiFine.") {
    this->keybind = 0;
    g_zoomMod = this;
}

ZoomModule::~ZoomModule() {
    if (g_zoomMod == this) g_zoomMod = nullptr;
}

void ZoomModule::onInit() {
    if (!m_fovHooked) {
        uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::GetFov);
        if (addr != 0) {
            ::bedrocktc::hooks::install((void*)addr, (void*)_getFov_zoom_hook, (void**)&_getFov_orig);
            m_fovHooked = true;
        }
    }
    
    if (!m_turnDeltaHooked) {
        uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::LocalPlayerApplyTurnDelta);
        if (addr != 0) {
            ::bedrocktc::hooks::install((void*)addr, (void*)_applyTurnDelta_hook, (void**)&_applyTurnDelta_orig);
            m_turnDeltaHooked = true;
        }
    }
    
    if (!m_hideHandHooked) {
        uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::BaseOptionRegistryGetHideItemInHand);
        if (addr != 0) {
            ::bedrocktc::hooks::install((void*)addr, (void*)_getHideItemInHand_hook, (void**)&_getHideItemInHand_orig);
            m_hideHandHooked = true;
        }
    }
    
    updateZoomButton();
}

void ZoomModule::onEnable() {
    m_isFirstTime = true;
    m_animationFinished = false;
}

void ZoomModule::onDisable() {
    m_animationFinished = false;
    m_keyZooming = false;
    m_buttonZooming = false;
}

bool ZoomModule::isZoomActive() {
    if (!enabled) return false;
    return m_keyZooming || m_buttonZooming;
}

void ZoomModule::onKeybindEvent(const std::string& key, bool isDown) {
    if (key == "keybind") {
        if (isDown && !m_keyZooming) {
            m_isFirstTime = true;
            m_animationFinished = false;
        }
        m_keyZooming = isDown;
    }
}

void ZoomModule::loadConfig(const nlohmann::json& j) {
    Module::loadConfig(j);
    if (j.contains("m_targetZoomFov")) m_targetZoomFov = j["m_targetZoomFov"].get<float>();
    if (j.contains("m_animSpeed")) m_animSpeed = j["m_animSpeed"].get<float>();
    if (j.contains("m_lowSens")) m_lowSens = j["m_lowSens"].get<bool>();
    if (j.contains("m_lowSensStrength")) m_lowSensStrength = j["m_lowSensStrength"].get<float>();
    if (j.contains("m_hideHand")) m_hideHand = j["m_hideHand"].get<bool>();
    if (j.contains("m_overlayToggle")) m_overlayToggle = j["m_overlayToggle"].get<bool>();
    
    updateZoomButton();
}

void ZoomModule::saveConfig(nlohmann::json& j) {
    Module::saveConfig(j);
    j["m_targetZoomFov"] = m_targetZoomFov;
    j["m_animSpeed"] = m_animSpeed;
    j["m_lowSens"] = m_lowSens;
    j["m_lowSensStrength"] = m_lowSensStrength;
    j["m_hideHand"] = m_hideHand;
    j["m_overlayToggle"] = m_overlayToggle;
}

void ZoomModule::updateZoomButton() {
    // The old launcher-side Zoom button belonged to LeviLauncher ModMenu.
    // BTP Menu owns module controls now, so no external button is registered.
}

} // namespace bedrocktc
