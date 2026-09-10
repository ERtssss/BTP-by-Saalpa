#include "weatherchanger.hpp"
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/world/Actor.hpp>
#include <bedrocktc/sdk/world/Weather.hpp>
#include <bedrocktc/events/EventBus.hpp>
#include <bedrocktc/hooks/Hooks.hpp>
#include <atomic>

namespace bedrocktc {



static WeatherChangerModule* g_weatherMod = nullptr;
static std::atomic<::bedrocktc::sdk::Weather*> g_lastWeather{nullptr};

using TickFn = void(*)(void*);
using WeatherLevelFn = float(*)(void*, float);
using WeatherStateFn = bool(*)(void*);
using BiomeGetTemperatureFn = float(*)(void*, void*, void*);

static TickFn s_origDimensionTick = nullptr;
static TickFn s_origWeatherTick = nullptr;
static WeatherLevelFn s_origGetRainLevel = nullptr;
static WeatherLevelFn s_origGetLightningLevel = nullptr;
static WeatherStateFn s_origIsRaining = nullptr;
static WeatherStateFn s_origIsLightning = nullptr;
static BiomeGetTemperatureFn s_origBiomeGetTemperature = nullptr;

static ::bedrocktc::hooks::Handle s_dimensionTickHookHandle = nullptr;
static ::bedrocktc::hooks::Handle s_weatherTickHookHandle = nullptr;
static ::bedrocktc::hooks::Handle s_getRainLevelHookHandle = nullptr;
static ::bedrocktc::hooks::Handle s_getLightningLevelHookHandle = nullptr;
static ::bedrocktc::hooks::Handle s_isRainingHookHandle = nullptr;
static ::bedrocktc::hooks::Handle s_isLightningHookHandle = nullptr;
static ::bedrocktc::hooks::Handle s_biomeTemperatureHookHandle = nullptr;

static void forceWeather(::bedrocktc::sdk::Weather* weather) {
    if (!weather) return;
    g_lastWeather.store(weather, std::memory_order_release);
    if (g_weatherMod && g_weatherMod->enabled) g_weatherMod->applyWeather(weather);
}

static void s_normalTickCallback(::bedrocktc::sdk::Player* localPlayer) {
    if (!localPlayer) return;
    auto* dimension = localPlayer->dimension();
    if (!dimension) return;
    forceWeather(dimension->weather());
}

static void s_dimensionTickHook(void* dimension) {
    auto* typedDimension = static_cast<::bedrocktc::sdk::Dimension*>(dimension);
    auto* weather = typedDimension ? typedDimension->weather() : nullptr;
    forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
    if (s_origDimensionTick) s_origDimensionTick(dimension);
    forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
}

static void s_weatherTickHook(void* weather) {
    forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
    if (s_origWeatherTick) s_origWeatherTick(weather);
    forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
}

static float s_getRainLevelHook(void* weather, float delta) {
    if (g_weatherMod && g_weatherMod->enabled) {
        forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
        return g_weatherMod->getRainLevel();
    }
    return s_origGetRainLevel ? s_origGetRainLevel(weather, delta) : 0.0f;
}

static float s_getLightningLevelHook(void* weather, float delta) {
    if (g_weatherMod && g_weatherMod->enabled) {
        forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
        return g_weatherMod->getLightningLevel();
    }
    return s_origGetLightningLevel ? s_origGetLightningLevel(weather, delta) : 0.0f;
}

static bool s_isRainingHook(void* weather) {
    if (g_weatherMod && g_weatherMod->enabled) {
        forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
        return g_weatherMod->getRainLevel() > 0.2f;
    }
    return s_origIsRaining ? s_origIsRaining(weather) : false;
}

static bool s_isLightningHook(void* weather) {
    if (g_weatherMod && g_weatherMod->enabled) {
        forceWeather(static_cast<::bedrocktc::sdk::Weather*>(weather));
        return g_weatherMod->getLightningLevel() > 0.2f;
    }
    return s_origIsLightning ? s_origIsLightning(weather) : false;
}

static float s_biomeGetTemperatureHook(void* biome, void* blockSource, void* blockPos) {
    if (g_weatherMod && g_weatherMod->enabled && g_weatherMod->m_mode == WeatherChangerModule::WeatherMode::Snow) {
        return 0.0f;
    }
    return s_origBiomeGetTemperature ? s_origBiomeGetTemperature(biome, blockSource, blockPos) : 0.5f;
}

static ::bedrocktc::hooks::Handle installHook(::bedrocktc::memory::SignatureId id, void* detour, void** original, ::bedrocktc::hooks::Handle& handle) {
    if (handle) return handle;
    auto address = ::bedrocktc::memory::resolve(id);
    if (!address) return nullptr;
    handle = ::bedrocktc::hooks::install(reinterpret_cast<void*>(address), detour, original);
    return handle;
}

WeatherChangerModule::WeatherChangerModule()
    : Module("Weather Changer", "Force clear, rain, thunder, or snow locally.") {
    g_weatherMod = this;
}

WeatherChangerModule::~WeatherChangerModule() {
    if (g_weatherMod == this) g_weatherMod = nullptr;
}

void WeatherChangerModule::installHooks() {
    installHook(::bedrocktc::memory::SignatureId::DimensionTick, reinterpret_cast<void*>(s_dimensionTickHook), reinterpret_cast<void**>(&s_origDimensionTick), s_dimensionTickHookHandle);
    installHook(::bedrocktc::memory::SignatureId::WeatherTick, reinterpret_cast<void*>(s_weatherTickHook), reinterpret_cast<void**>(&s_origWeatherTick), s_weatherTickHookHandle);
    installHook(::bedrocktc::memory::SignatureId::WeatherGetRainLevel, reinterpret_cast<void*>(s_getRainLevelHook), reinterpret_cast<void**>(&s_origGetRainLevel), s_getRainLevelHookHandle);
    installHook(::bedrocktc::memory::SignatureId::WeatherGetLightningLevel, reinterpret_cast<void*>(s_getLightningLevelHook), reinterpret_cast<void**>(&s_origGetLightningLevel), s_getLightningLevelHookHandle);
    installHook(::bedrocktc::memory::SignatureId::WeatherIsRaining, reinterpret_cast<void*>(s_isRainingHook), reinterpret_cast<void**>(&s_origIsRaining), s_isRainingHookHandle);
    installHook(::bedrocktc::memory::SignatureId::WeatherIsLightning, reinterpret_cast<void*>(s_isLightningHook), reinterpret_cast<void**>(&s_origIsLightning), s_isLightningHookHandle);
    installHook(::bedrocktc::memory::SignatureId::BiomeGetTemperature, reinterpret_cast<void*>(s_biomeGetTemperatureHook), reinterpret_cast<void**>(&s_origBiomeGetTemperature), s_biomeTemperatureHookHandle);

    m_hooked = s_dimensionTickHookHandle || s_weatherTickHookHandle || s_getRainLevelHookHandle || s_getLightningLevelHookHandle;
}

void WeatherChangerModule::onInit() {
    installHooks();
    ::bedrocktc::events::bus().subscribe<::bedrocktc::events::LocalPlayerTickEvent>([](auto& event) { s_normalTickCallback(event.player); });
}

void WeatherChangerModule::onEnable() {
    if (!m_hooked) installHooks();
    applyWeather(g_lastWeather.load(std::memory_order_acquire));
}

void WeatherChangerModule::onDisable() {}

float WeatherChangerModule::getRainLevel() const {
    switch (m_mode) {
        case WeatherMode::Rain:
        case WeatherMode::Thunder:
        case WeatherMode::Snow:
            return 1.0f;
        case WeatherMode::Clear:
        default:
            return 0.0f;
    }
}

float WeatherChangerModule::getLightningLevel() const {
    return m_mode == WeatherMode::Thunder ? 1.0f : 0.0f;
}

void WeatherChangerModule::applyWeather(void* weather) const {
    auto* state = static_cast<::bedrocktc::sdk::Weather*>(weather);
    if (!state) return;
    state->setRainLevel(getRainLevel());
    state->setLightningLevel(getLightningLevel());
}

void WeatherChangerModule::loadConfig(const nlohmann::json& j) {
    Module::loadConfig(j);
    if (!j.contains("mode")) return;

    const auto& value = j["mode"];
    if (value.is_string()) {
        auto text = value.get<std::string>();
        auto comma = text.find(',');
        try {
            int mode = std::stoi(text.substr(0, comma));
            if (mode >= static_cast<int>(WeatherMode::Clear) && mode <= static_cast<int>(WeatherMode::Snow)) {
                m_mode = static_cast<WeatherMode>(mode);
            }
        } catch (...) {}
    } else if (value.is_number_integer()) {
        int mode = value.get<int>();
        if (mode >= static_cast<int>(WeatherMode::Clear) && mode <= static_cast<int>(WeatherMode::Snow)) {
            m_mode = static_cast<WeatherMode>(mode);
        }
    }

    if (enabled) applyWeather(g_lastWeather.load(std::memory_order_acquire));
}

void WeatherChangerModule::saveConfig(nlohmann::json& j) {
    Module::saveConfig(j);
    j["mode"] = std::to_string(static_cast<int>(m_mode)) + ",Clear,Rain,Thunder,Snow";
}

} // namespace bedrocktc
