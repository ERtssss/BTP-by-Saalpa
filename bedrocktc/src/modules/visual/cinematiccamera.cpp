#include "cinematiccamera.hpp"
#include <bedrocktc/hooks/Hooks.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <pl/ModMenu.hpp>
#include <algorithm>
#include <cmath>
#include <span>
#include <vector>

namespace bedrocktc {




namespace {

using TurnDeltaFn = void (*)(void*, ::bedrocktc::sdk::Vec2*);

CinematicCameraModule* moduleInstance = nullptr;
TurnDeltaFn turnDeltaOriginal = nullptr;
::bedrocktc::hooks::Handle turnDeltaHook = nullptr;

void turnDeltaDetour(void* self, ::bedrocktc::sdk::Vec2* delta) {
    if (!turnDeltaOriginal) return;
    if (!delta || !moduleInstance) {
        turnDeltaOriginal(self, delta);
        return;
    }

    ::bedrocktc::sdk::Vec2 modified = *delta;
    moduleInstance->applyTurnDelta(modified);
    turnDeltaOriginal(self, &modified);
}

}

CinematicCameraModule::CinematicCameraModule()
    : Module("Cinematic Camera", "Smooth cinema-like camera movement with optional cinematic bars.") {
    keybind = 0;
    moduleInstance = this;
}

CinematicCameraModule::~CinematicCameraModule() {
    if (turnDeltaHook) {
        ::bedrocktc::hooks::remove(turnDeltaHook);
        turnDeltaHook = nullptr;
        turnDeltaOriginal = nullptr;
    }
    if (moduleInstance == this) moduleInstance = nullptr;
}

void CinematicCameraModule::onInit() {
    if (turnDeltaHook) return;
    const uintptr_t address = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::LocalPlayerApplyTurnDelta);
    if (!address) return;
    turnDeltaHook = ::bedrocktc::hooks::install(
        reinterpret_cast<void*>(address),
        reinterpret_cast<void*>(turnDeltaDetour),
        reinterpret_cast<void**>(&turnDeltaOriginal));
}

void CinematicCameraModule::onEnable() {
    {
        std::lock_guard lock(m_stateMutex);
        m_cameraActive = false;
        m_keyDown = false;
    }
    resetSmoothing();
}

void CinematicCameraModule::onDisable() {
    {
        std::lock_guard lock(m_stateMutex);
        m_cameraActive = false;
        m_keyDown = false;
    }
    resetSmoothing();
    clearBars();
}

void CinematicCameraModule::onKeybindEvent(const std::string& key, bool isDown) {
    if (key != "keybind") return;

    const ConfigSnapshot config = snapshotConfig();
    std::lock_guard lock(m_stateMutex);

    if (!config.toggleMode) {
        m_keyDown = isDown;
        return;
    }

    if (isDown && !m_keyDown) {
        m_cameraActive = !m_cameraActive;
        m_smoothDelta = {0.0f, 0.0f};
        m_lastTime = std::chrono::steady_clock::now();
    }
    m_keyDown = isDown;
}

CinematicCameraModule::ConfigSnapshot CinematicCameraModule::snapshotConfig() const {
    std::lock_guard lock(m_configMutex);
    return {
        m_toggleMode,
        m_smoothing,
        std::clamp(m_smoothness, 0.0f, 10.0f),
        m_cinebars,
        std::clamp(m_cinebarHeight, 0.0f, 0.8f),
        parseColor(m_cinebarColor, 0xFF000000u)
    };
}

bool CinematicCameraModule::effectActive(const ConfigSnapshot& config) const {
    if (!enabled) return false;
    if (!config.toggleMode) return true;
    std::lock_guard lock(m_stateMutex);
    return m_cameraActive;
}

void CinematicCameraModule::resetSmoothing() {
    std::lock_guard lock(m_stateMutex);
    m_smoothDelta = {0.0f, 0.0f};
    m_lastTime = std::chrono::steady_clock::now();
}

void CinematicCameraModule::applyTurnDelta(::bedrocktc::sdk::Vec2& delta) {
    if (!enabled) return;

    const ConfigSnapshot config = snapshotConfig();
    if (!config.smoothing) {
        resetSmoothing();
        return;
    }

    std::lock_guard lock(m_stateMutex);
    if (config.toggleMode && !m_cameraActive) {
        m_smoothDelta = {0.0f, 0.0f};
        m_lastTime = std::chrono::steady_clock::now();
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
    m_lastTime = now;
    deltaTime = std::clamp(deltaTime, 0.0f, 0.25f);

    const float response = std::max(0.01f, 10.01f - config.smoothness);
    const float alpha = std::clamp(1.0f - std::exp(-deltaTime * response), 0.0f, 1.0f);

    m_smoothDelta.x += (delta.x - m_smoothDelta.x) * alpha;
    m_smoothDelta.y += (delta.y - m_smoothDelta.y) * alpha;
    delta = m_smoothDelta;
}

void CinematicCameraModule::onFrame() {
    const ConfigSnapshot config = snapshotConfig();
    if (!config.cinebars || !effectActive(config)) {
        clearBars();
        return;
    }

    const float surfaceWidth = 1200.0f;
    const float surfaceHeight = 720.0f;
    const float barHeight = surfaceHeight * config.cinebarHeight * 0.5f;
    if (barHeight <= 0.0f) {
        clearBars();
        return;
    }

    std::vector<pl::modmenu::DrawCommand> commands;
    commands.reserve(2);

    pl::modmenu::DrawCommand top{};
    top.type = pl::modmenu::DrawCommandType::RectFilled;
    top.x = 0.0f;
    top.y = 0.0f;
    top.w = surfaceWidth;
    top.h = barHeight;
    top.color = config.cinebarColor;
    commands.push_back(std::move(top));

    pl::modmenu::DrawCommand bottom{};
    bottom.type = pl::modmenu::DrawCommandType::RectFilled;
    bottom.x = 0.0f;
    bottom.y = surfaceHeight - barHeight;
    bottom.w = surfaceWidth;
    bottom.h = barHeight;
    bottom.color = config.cinebarColor;
    commands.push_back(std::move(bottom));

    pl::modmenu::submitDrawCommands(moduleId, commands);
}

void CinematicCameraModule::clearBars() {
    pl::modmenu::submitDrawCommands(moduleId, std::span<const pl::modmenu::DrawCommand>{});
}

void CinematicCameraModule::onMenuRegistered() {
    using namespace pl::modmenu;
    ModuleBuilder builder(moduleId, name);
    builder.modId("btp").description(description).defaultEnabled(masterEnabled)
        .config("masterEnabled", "Enabled", ConfigType::Toggle, masterEnabled ? "true" : "false")
        .config("keybind", "Keybind", ConfigType::SliderInt, std::to_string(keybind), "0", "255")
        .onToggle([this](std::string_view, bool enabledValue){ setMasterEnabled(enabledValue); });
    builder.registerModule();
}

void CinematicCameraModule::loadConfig(const nlohmann::json& j) {
    Module::loadConfig(j);

    bool toggleChanged = false;
    bool smoothingChanged = false;
    {
        std::lock_guard lock(m_configMutex);
        if (j.contains("m_toggleMode")) {
            const bool value = j["m_toggleMode"].get<bool>();
            toggleChanged = value != m_toggleMode;
            m_toggleMode = value;
        }
        if (j.contains("m_smoothing")) {
            const bool value = j["m_smoothing"].get<bool>();
            smoothingChanged = value != m_smoothing;
            m_smoothing = value;
        }
        if (j.contains("m_smoothness")) m_smoothness = std::clamp(j["m_smoothness"].get<float>(), 0.0f, 10.0f);
        if (j.contains("m_cinebars")) m_cinebars = j["m_cinebars"].get<bool>();
        if (j.contains("m_cinebarHeight")) m_cinebarHeight = std::clamp(j["m_cinebarHeight"].get<float>(), 0.0f, 0.8f);
        if (j.contains("m_cinebarColor")) m_cinebarColor = j["m_cinebarColor"].get<std::string>();
    }

    if (toggleChanged) {
        std::lock_guard lock(m_stateMutex);
        m_cameraActive = false;
        m_keyDown = false;
        m_smoothDelta = {0.0f, 0.0f};
        m_lastTime = std::chrono::steady_clock::now();
    } else if (smoothingChanged) {
        resetSmoothing();
    }
}

void CinematicCameraModule::saveConfig(nlohmann::json& j) {
    Module::saveConfig(j);

    std::lock_guard lock(m_configMutex);
    j["m_toggleMode"] = m_toggleMode;
    j["m_smoothing"] = m_smoothing;
    j["m_smoothness"] = m_smoothness;
    j["m_cinebars"] = m_cinebars;
    j["m_cinebarHeight"] = m_cinebarHeight;
    j["m_cinebarColor"] = m_cinebarColor;
}

std::uint32_t CinematicCameraModule::parseColor(const std::string& value, std::uint32_t fallback) {
    if (value.empty()) return fallback;
    const std::string hex = value[0] == '#' ? value.substr(1) : value;
    try {
        if (hex.size() == 6) return 0xFF000000u | static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
        if (hex.size() == 8) return static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
    } catch (...) {
    }
    return fallback;
}

} // namespace bedrocktc
