#include "potionhud.hpp"
#include "potionhud_assets.hpp"
#include <bedrocktc/hooks/Hooks.hpp>
#include "modules/ModuleRegistry.hpp"
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/input/MoveInput.hpp>
#include <bedrocktc/sdk/world/MobEffects.hpp>
#include <pl/ModMenu.hpp>
#include <pl/memory/Vtable.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bedrocktc {




namespace {

constexpr const char* MinecraftLibrary = "libminecraftpe.so";
constexpr std::size_t MaxEffects = 64;
constexpr float VanillaEffectSize = 16.0f;
constexpr int WarningSeconds = 5;
constexpr const char* InstantHealthImageId = "bedrocktc.potionhud.instant_health";
constexpr const char* InstantDamageImageId = "bedrocktc.potionhud.instant_damage";
constexpr const char* SaturationImageId = "bedrocktc.potionhud.saturation";
constexpr const char* GenericPotionImageId = "bedrocktc.potionhud.generic";
struct RectangleArea {
    float x0;
    float x1;
    float y0;
    float y1;
};

struct UiVec2 {
    float x;
    float y;
};

struct Color {
    float r;
    float g;
    float b;
    float a;
};

struct ClientTexture {
    std::byte storage[24]{};
};

struct BedrockTextureData {
    ClientTexture clientTexture;
};

enum class ResourceFileSystem : int {
    UserPackage = 0
};

class ResourceLocation {
public:
    ResourceFileSystem fileSystem;
    std::string path;
    std::uint64_t pathHash;
    std::uint64_t fullHash;

    explicit ResourceLocation(std::string_view value)
        : fileSystem(ResourceFileSystem::UserPackage),
          path(value),
          pathHash(computeHash(path)),
          fullHash(pathHash ^ static_cast<std::uint64_t>(fileSystem)) {}

private:
    static std::uint64_t computeHash(std::string_view value) {
        constexpr std::uint64_t offset = 1469598103934665603ULL;
        constexpr std::uint64_t prime = 1099511628211ULL;
        std::uint64_t hash = offset;
        for (unsigned char ch : value) hash = static_cast<std::uint64_t>(ch) ^ (prime * hash);
        return hash;
    }
};

class TexturePtr {
public:
    std::shared_ptr<const BedrockTextureData> clientTexture;
    std::shared_ptr<ResourceLocation> resourceLocation;

    const ClientTexture& getClientTexture() const {
        static const ClientTexture empty{};
        return clientTexture ? clientTexture->clientTexture : empty;
    }
};

class HashedString {
public:
    std::uint64_t hash;
    std::string value;
    mutable const HashedString* lastMatch;

    explicit HashedString(const char* text)
        : hash(computeHash(text ? std::string_view(text) : std::string_view())),
          value(text ? text : ""),
          lastMatch(nullptr) {}

private:
    static std::uint64_t computeHash(std::string_view text) {
        if (text.empty()) return 0;
        constexpr std::uint64_t offset = 0xCBF29CE484222325ULL;
        constexpr std::uint64_t prime = 0x100000001B3ULL;
        std::uint64_t result = offset;
        for (char character : text) {
            result = static_cast<std::uint64_t>(static_cast<unsigned char>(character)) ^ (prime * result);
        }
        return result;
    }
};

struct RawEffect {
    std::uint32_t id;
    int duration;
    int amplifier;
    bool noCounter;
};

using HudMobEffectsRendererFn = void* (*)(void*, void*, void*, void*, int, void*);

HudMobEffectsRendererFn hudMobEffectsRendererOriginal = nullptr;
PotionHudModule* moduleInstance = nullptr;
::bedrocktc::hooks::Handle hudMobEffectsRendererHook = nullptr;

void** getVtable(void* object) {
    return object ? *reinterpret_cast<void***>(object) : nullptr;
}

void* getLocalPlayer(void* client) {
    void** vtable = getVtable(client);
    if (!vtable || !vtable[::bedrocktc::sdk::offsets::VTable::ClientInstanceGetLocalPlayer]) return nullptr;
    return reinterpret_cast<void* (*)(void*)>(vtable[::bedrocktc::sdk::offsets::VTable::ClientInstanceGetLocalPlayer])(client);
}

MobEffectsComponent* getMobEffects(void* player) {
    if (!player) return nullptr;
    auto* context = reinterpret_cast<EntityContext*>(
        reinterpret_cast<std::uintptr_t>(player) + ::bedrocktc::sdk::offsets::Actor::mEntityContext);
    return context->tryGetComponent<MobEffectsComponent>();
}

bool copyEffects(MobEffectsComponent* component, std::vector<RawEffect>& out) {
    if (!component) return false;
    const auto begin = reinterpret_cast<std::uintptr_t>(component->begin);
    const auto end = reinterpret_cast<std::uintptr_t>(component->end);
    const auto capacity = reinterpret_cast<std::uintptr_t>(component->capacity);
    if (begin == 0 && end == 0 && capacity == 0) return true;
    if (!begin || !end || !capacity || end < begin || capacity < end) return false;
    const auto span = end - begin;
    const auto capacitySpan = capacity - begin;
    if (span % sizeof(MobEffectInstance) != 0 || capacitySpan % sizeof(MobEffectInstance) != 0) return false;
    const std::size_t count = span / sizeof(MobEffectInstance);
    const std::size_t capacityCount = capacitySpan / sizeof(MobEffectInstance);
    if (count > MaxEffects || capacityCount > MaxEffects * 4 || count > capacityCount) return false;
    out.clear();
    out.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const auto* effect = reinterpret_cast<const MobEffectInstance*>(begin + i * sizeof(MobEffectInstance));
        const auto id = static_cast<std::uint32_t>(effect->id);
        if (id == 0 || id > static_cast<std::uint32_t>(MobEffectType::BreathOfTheNautilus)) continue;
        if (effect->duration < -1 || (effect->duration == 0 && !effect->noCounter)) continue;
        out.push_back({id, effect->duration, std::clamp(effect->amplifier, 0, 255), effect->noCounter});
    }
    return true;
}

RectangleArea getFullClippingRectangle(void* context) {
    RectangleArea result{};
    void** vtable = getVtable(context);
    if (!vtable || !vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextGetFullClippingRectangle]) return result;
    using Fn = RectangleArea (*)(void*);
    return reinterpret_cast<Fn>(vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextGetFullClippingRectangle])(context);
}

bool validRectangle(const RectangleArea& area) {
    return std::isfinite(area.x0) && std::isfinite(area.x1) && std::isfinite(area.y0) && std::isfinite(area.y1) &&
           area.x1 > area.x0 && area.y1 > area.y0;
}

TexturePtr getTexture(void* context, const ResourceLocation& location) {
    void** vtable = getVtable(context);
    if (!vtable || !vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextGetTexture]) return {};
    using Fn = TexturePtr (*)(void*, const ResourceLocation&, bool);
    return reinterpret_cast<Fn>(vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextGetTexture])(context, location, false);
}

void drawImage(void* context, const ClientTexture& texture, const UiVec2& position, const UiVec2& size) {
    void** vtable = getVtable(context);
    if (!vtable || !vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextDrawImage]) return;
    using Fn = void (*)(void*, const ClientTexture&, const UiVec2&, const UiVec2&, const UiVec2&, const UiVec2&, bool);
    static constexpr UiVec2 uv{0.0f, 0.0f};
    static constexpr UiVec2 uvSize{1.0f, 1.0f};
    reinterpret_cast<Fn>(vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextDrawImage])(
        context, texture, position, size, uv, uvSize, false);
}

void flushImages(void* context) {
    void** vtable = getVtable(context);
    if (!vtable || !vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextFlushImages]) return;
    using Fn = void (*)(void*, const Color&, float, const HashedString&);
    static const HashedString material("ui_flush");
    static constexpr Color color{1.0f, 1.0f, 1.0f, 1.0f};
    reinterpret_cast<Fn>(vtable[::bedrocktc::sdk::offsets::VTable::MinecraftUIRenderContextFlushImages])(
        context, color, 1.0f, material);
}

std::uint32_t parseColor(const std::string& value, std::uint32_t fallback) {
    if (value.empty()) return fallback;
    const std::string hex = value[0] == '#' ? value.substr(1) : value;
    try {
        if (hex.size() == 6) return 0xFF000000u | static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
        if (hex.size() == 8) return static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
    } catch (...) {
    }
    return fallback;
}

std::string_view effectName(std::uint32_t id) {
    switch (static_cast<MobEffectType>(id)) {
        case MobEffectType::Speed: return "Speed";
        case MobEffectType::Slowness: return "Slowness";
        case MobEffectType::Haste: return "Haste";
        case MobEffectType::MiningFatigue: return "Mining Fatigue";
        case MobEffectType::Strength: return "Strength";
        case MobEffectType::InstantHealth: return "Instant Health";
        case MobEffectType::InstantDamage: return "Instant Damage";
        case MobEffectType::JumpBoost: return "Jump Boost";
        case MobEffectType::Nausea: return "Nausea";
        case MobEffectType::Regeneration: return "Regeneration";
        case MobEffectType::Resistance: return "Resistance";
        case MobEffectType::FireResistance: return "Fire Resistance";
        case MobEffectType::WaterBreathing: return "Water Breathing";
        case MobEffectType::Invisibility: return "Invisibility";
        case MobEffectType::Blindness: return "Blindness";
        case MobEffectType::NightVision: return "Night Vision";
        case MobEffectType::Hunger: return "Hunger";
        case MobEffectType::Weakness: return "Weakness";
        case MobEffectType::Poison: return "Poison";
        case MobEffectType::Wither: return "Wither";
        case MobEffectType::HealthBoost: return "Health Boost";
        case MobEffectType::Absorption: return "Absorption";
        case MobEffectType::Saturation: return "Saturation";
        case MobEffectType::Levitation: return "Levitation";
        case MobEffectType::FatalPoison: return "Fatal Poison";
        case MobEffectType::ConduitPower: return "Conduit Power";
        case MobEffectType::SlowFalling: return "Slow Falling";
        case MobEffectType::BadOmen: return "Bad Omen";
        case MobEffectType::VillageHero: return "Village Hero";
        case MobEffectType::Darkness: return "Darkness";
        case MobEffectType::TrialOmen: return "Trial Omen";
        case MobEffectType::WindCharged: return "Wind Charged";
        case MobEffectType::Weaving: return "Weaving";
        case MobEffectType::Oozing: return "Oozing";
        case MobEffectType::Infested: return "Infested";
        case MobEffectType::RaidOmen: return "Raid Omen";
        case MobEffectType::BreathOfTheNautilus: return "Breath of the Nautilus";
        default: return "Unknown Effect";
    }
}

std::string_view effectTexturePath(std::uint32_t id) {
    switch (static_cast<MobEffectType>(id)) {
        case MobEffectType::Speed: return "textures/ui/speed_effect";
        case MobEffectType::Slowness: return "textures/ui/slowness_effect";
        case MobEffectType::Haste: return "textures/ui/haste_effect";
        case MobEffectType::MiningFatigue: return "textures/ui/mining_fatigue_effect";
        case MobEffectType::Strength: return "textures/ui/strength_effect";
        case MobEffectType::InstantHealth: return "textures/ui/instant_health_effect";
        case MobEffectType::InstantDamage: return "textures/ui/instant_damage_effect";
        case MobEffectType::JumpBoost: return "textures/ui/jump_boost_effect";
        case MobEffectType::Nausea: return "textures/ui/nausea_effect";
        case MobEffectType::Regeneration: return "textures/ui/regeneration_effect";
        case MobEffectType::Resistance: return "textures/ui/resistance_effect";
        case MobEffectType::FireResistance: return "textures/ui/fire_resistance_effect";
        case MobEffectType::WaterBreathing: return "textures/ui/water_breathing_effect";
        case MobEffectType::Invisibility: return "textures/ui/invisibility_effect";
        case MobEffectType::Blindness: return "textures/ui/blindness_effect";
        case MobEffectType::NightVision: return "textures/ui/night_vision_effect";
        case MobEffectType::Hunger: return "textures/ui/hunger_effect";
        case MobEffectType::Weakness: return "textures/ui/weakness_effect";
        case MobEffectType::Poison: return "textures/ui/poison_effect";
        case MobEffectType::Wither: return "textures/ui/wither_effect";
        case MobEffectType::HealthBoost: return "textures/ui/health_boost_effect";
        case MobEffectType::Absorption: return "textures/ui/absorption_effect";
        case MobEffectType::Saturation: return "textures/ui/saturation_effect";
        case MobEffectType::Levitation: return "textures/ui/levitation_effect";
        case MobEffectType::FatalPoison: return "textures/ui/fatal_poison_effect";
        case MobEffectType::ConduitPower: return "textures/ui/conduit_power_effect";
        case MobEffectType::SlowFalling: return "textures/ui/slow_falling_effect";
        case MobEffectType::BadOmen: return "textures/ui/bad_omen_effect";
        case MobEffectType::VillageHero: return "textures/ui/village_hero_effect";
        case MobEffectType::Darkness: return "textures/ui/darkness_effect";
        case MobEffectType::TrialOmen: return "textures/ui/trial_omen_effect";
        case MobEffectType::WindCharged: return "textures/ui/wind_charged_effect";
        case MobEffectType::Weaving: return "textures/ui/weaving_effect";
        case MobEffectType::Oozing: return "textures/ui/oozing_effect";
        case MobEffectType::Infested: return "textures/ui/infested_effect";
        case MobEffectType::RaidOmen: return "textures/ui/raid_omen_effect";
        case MobEffectType::BreathOfTheNautilus: return "textures/ui/breath_of_the_nautilus_effect";
        default: return {};
    }
}

bool usesNativeTexture(std::uint32_t id) {
    return id != static_cast<std::uint32_t>(MobEffectType::InstantHealth) &&
           id != static_cast<std::uint32_t>(MobEffectType::InstantDamage) &&
           id != static_cast<std::uint32_t>(MobEffectType::Saturation);
}

const char* fallbackImageId(std::uint32_t id) {
    if (id == static_cast<std::uint32_t>(MobEffectType::InstantHealth)) return InstantHealthImageId;
    if (id == static_cast<std::uint32_t>(MobEffectType::InstantDamage)) return InstantDamageImageId;
    if (id == static_cast<std::uint32_t>(MobEffectType::Saturation)) return SaturationImageId;
    return GenericPotionImageId;
}

std::string romanNumeral(int value) {
    if (value <= 0 || value > 3999) return std::to_string(value);
    static constexpr std::pair<int, std::string_view> table[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"}, {90, "XC"},
        {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
    };
    std::string result;
    for (const auto& [number, numeral] : table) {
        while (value >= number) {
            result += numeral;
            value -= number;
        }
    }
    return result;
}

std::string compactRoman(int value) {
    switch (value) {
        case 1: return "I";
        case 2: return "II";
        case 3: return "III";
        case 4: return "IV";
        case 5: return "V";
        default: return std::to_string(value);
    }
}

std::string formatDuration(int duration, bool noCounter) {
    if (noCounter || duration == -1) return "\xE2\x88\x9E";
    const int totalSeconds = std::max(0, duration / 20);
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    std::string result = std::to_string(minutes) + ":";
    if (seconds < 10) result += '0';
    result += std::to_string(seconds);
    return result;
}

void* hudMobEffectsRendererDetour(void* renderer, void* context, void* client, void* owner, int pass, void* renderAabb) {
    if (moduleInstance && moduleInstance->enabled && moduleInstance->renderNative(context, client)) return nullptr;
    if (!hudMobEffectsRendererOriginal) return nullptr;
    return hudMobEffectsRendererOriginal(renderer, context, client, owner, pass, renderAabb);
}

}

PotionHudModule::PotionHudModule()
    : Module("PotionHUD", "Displays active potion effects with icons, names, amplifiers, and timers.") {
    moduleInstance = this;
}

PotionHudModule::~PotionHudModule() {
    if (hudMobEffectsRendererHook) {
        ::bedrocktc::hooks::remove(hudMobEffectsRendererHook);
        hudMobEffectsRendererHook = nullptr;
        hudMobEffectsRendererOriginal = nullptr;
    }
    if (moduleInstance == this) moduleInstance = nullptr;
}

void PotionHudModule::onInit() {
    pl::modmenu::registerImage(InstantHealthImageId, potionhud_assets::InstantHealthPixels, potionhud_assets::InstantHealthWidth, potionhud_assets::InstantHealthHeight);
    pl::modmenu::registerImage(InstantDamageImageId, potionhud_assets::InstantDamagePixels, potionhud_assets::InstantDamageWidth, potionhud_assets::InstantDamageHeight);
    pl::modmenu::registerImage(SaturationImageId, potionhud_assets::SaturationPixels, potionhud_assets::SaturationWidth, potionhud_assets::SaturationHeight);
    pl::modmenu::registerImage(GenericPotionImageId, potionhud_assets::GenericPotionPixels, potionhud_assets::GenericPotionWidth, potionhud_assets::GenericPotionHeight);

    const std::uintptr_t renderer = pl::memory::resolveVtableFunction(
        "21HudMobEffectsRenderer",
        ::bedrocktc::sdk::offsets::VTable::HudMobEffectsRendererRender,
        MinecraftLibrary);
    if (renderer && !hudMobEffectsRendererHook) {
        hudMobEffectsRendererHook = ::bedrocktc::hooks::install(
            reinterpret_cast<void*>(renderer),
            reinterpret_cast<void*>(hudMobEffectsRendererDetour),
            reinterpret_cast<void**>(&hudMobEffectsRendererOriginal));
    }
}

void PotionHudModule::onDisable() {
    std::lock_guard lock(m_runtimeMutex);
    m_runtimeEffects.clear();
    m_runtimeValid.store(false);
}

void PotionHudModule::onFrame() {
    // Runtime HUD data is populated by the native renderer hook when available.
}

bool PotionHudModule::renderNative(void* context, void* client) {
    (void)context;
    (void)client;
    // Keep the hook transparent until a valid native HUD surface is available.
    return false;
}

void PotionHudModule::onMenuRegistered() {
    using namespace pl::modmenu;
    ModuleBuilder builder(moduleId, name);
    builder.modId("btp").description(description).defaultEnabled(masterEnabled)
        .config("masterEnabled", "Enabled", ConfigType::Toggle, masterEnabled ? "true" : "false")
        .config("keybind", "Keybind", ConfigType::SliderInt, std::to_string(keybind), "0", "255")
        .onToggle([this](std::string_view, bool enabledValue){ setMasterEnabled(enabledValue); });
    builder.registerModule();
}

void PotionHudModule::loadConfig(const nlohmann::json& j) {
    Module::loadConfig(j);
    std::lock_guard lock(m_configMutex);
    if (j.contains("hudPosX")) hudPosX = std::clamp(j["hudPosX"].get<float>(), 0.0f, 4000.0f);
    if (j.contains("hudPosY")) hudPosY = std::clamp(j["hudPosY"].get<float>(), 0.0f, 4000.0f);
    if (j.contains("m_uiScale")) m_uiScale = std::clamp(j["m_uiScale"].get<float>(), 0.5f, 3.0f);
    if (j.contains("m_spacing")) m_spacing = std::clamp(j["m_spacing"].get<float>(), 0.25f, 3.0f);
    if (j.contains("m_bottomUp")) m_bottomUp = j["m_bottomUp"].get<bool>();
    if (j.contains("m_showText")) m_showText = j["m_showText"].get<bool>();
    if (j.contains("m_showTitle")) m_showTitle = j["m_showTitle"].get<bool>();
    if (j.contains("m_useRoman")) m_useRoman = j["m_useRoman"].get<bool>();
    if (j.contains("m_useRomanFull")) m_useRomanFull = j["m_useRomanFull"].get<bool>();
    if (j.contains("m_textSize")) m_textSize = std::clamp(j["m_textSize"].get<float>(), 4.0f, 20.0f);
    if (j.contains("m_textOffsetX")) m_textOffsetX = std::clamp(j["m_textOffsetX"].get<float>(), 0.0f, 20.0f);
    if (j.contains("m_textSide")) {
        try {
            std::string value = j["m_textSide"].get<std::string>();
            const std::size_t separator = value.find(',');
            if (separator != std::string::npos) value.resize(separator);
            m_textSide = std::clamp(std::stoi(value), 0, 1);
        } catch (...) {
        }
    }
    if (j.contains("m_textShadow")) m_textShadow = j["m_textShadow"].get<bool>();
    if (j.contains("m_shadowOffset")) m_shadowOffset = std::clamp(j["m_shadowOffset"].get<float>(), 0.25f, 5.0f);
    if (j.contains("m_mainColor")) m_mainColor = j["m_mainColor"].get<std::string>();
    if (j.contains("m_lowColor")) m_lowColor = j["m_lowColor"].get<std::string>();
    if (j.contains("m_shadowColor")) m_shadowColor = j["m_shadowColor"].get<std::string>();
    if (j.contains("m_gridSize")) m_gridSize = std::clamp(j["m_gridSize"].get<float>(), 1.0f, 100.0f);
    if (j.contains("m_gridGap")) m_gridGap = std::clamp(j["m_gridGap"].get<float>(), 0.0f, 100.0f);
    if (j.contains("m_snapThreshold")) m_snapThreshold = std::clamp(j["m_snapThreshold"].get<float>(), 1.0f, 100.0f);
    if (j.contains("m_snapToGrid")) m_snapToGrid = j["m_snapToGrid"].get<bool>();
    if (j.contains("m_snapToElements")) m_snapToElements = j["m_snapToElements"].get<bool>();
    if (j.contains("m_snapToScreenCenter")) m_snapToScreenCenter = j["m_snapToScreenCenter"].get<bool>();
}

void PotionHudModule::saveConfig(nlohmann::json& j) {
    Module::saveConfig(j);
    std::lock_guard lock(m_configMutex);
    j["isHudModule"] = true;
    j["hudPosX"] = hudPosX;
    j["hudPosY"] = hudPosY;
    j["m_uiScale"] = m_uiScale;
    j["m_spacing"] = m_spacing;
    j["m_bottomUp"] = m_bottomUp;
    j["m_showText"] = m_showText;
    j["m_showTitle"] = m_showTitle;
    j["m_useRoman"] = m_useRoman;
    j["m_useRomanFull"] = m_useRomanFull;
    j["m_textSize"] = m_textSize;
    j["m_textOffsetX"] = m_textOffsetX;
    j["m_textSide"] = std::to_string(m_textSide) + ",Right,Left";
    j["m_textShadow"] = m_textShadow;
    j["m_shadowOffset"] = m_shadowOffset;
    j["m_mainColor"] = m_mainColor;
    j["m_lowColor"] = m_lowColor;
    j["m_shadowColor"] = m_shadowColor;
    j["m_gridSize"] = m_gridSize;
    j["m_gridGap"] = m_gridGap;
    j["m_snapThreshold"] = m_snapThreshold;
    j["m_snapToGrid"] = m_snapToGrid;
    j["m_snapToElements"] = m_snapToElements;
    j["m_snapToScreenCenter"] = m_snapToScreenCenter;
}

} // namespace bedrocktc
