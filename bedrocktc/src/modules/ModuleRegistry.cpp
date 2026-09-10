#include "ModuleRegistry.hpp"
#include <algorithm>
#include <cctype>
#include <exception>
#include <nlohmann/json.hpp>
#include <btp/debug/Log.hpp>
#include "visual/fullbright.hpp"
#include "visual/motionblur.hpp"
#include "visual/fpsunlocker.hpp"
#include "visual/fogcolor.hpp"
#include "visual/glintcolor.hpp"
#include "visual/tnttimer.hpp"
#include "visual/nofog.hpp"
#include "hud/pingcounter.hpp"
#include "hud/worldtime.hpp"
#include "hud/totemcounter.hpp"
#include "hud/arrowcounter.hpp"
#include "hud/potcounter.hpp"
#include "player/quickloot.hpp"
#include "hud/reachcounter.hpp"
#include "hud/breakindicator.hpp"
#include "hud/playercoords.hpp"
#include "hud/compass.hpp"
#include "hud/armorhud.hpp"
#include "hud/potionhud.hpp"
#include "player/timechanger.hpp"
#include "player/autosprint.hpp"
#include "player/weatherchanger.hpp"
#include "player/nick.hpp"
#include "misc/nodisconnect.hpp"
#include "misc/chattimestamps.hpp"
#include "misc/notouchborder.hpp"
#include "misc/forceglobalrp.hpp"
#include "misc/cpslimiter.hpp"
#include "hud/speeddisplay.hpp"
#include "hud/debugmenu.hpp"
#include "visual/viewmodel.hpp"
#include "hud/keystrokes.hpp"
#include "visual/thirdpersonnametag.hpp"
#include "hud/tablist.hpp"
#include "hud/combocounter.hpp"
#include "visual/chunkborder.hpp"
#include "visual/hitbox.hpp"
#include "visual/zoom.hpp"
#include "visual/cinematiccamera.hpp"
#include "visual/breadcrumbs.hpp"
#include "visual/lightoverlay.hpp"
#include "visual/shulkerpreview.hpp"
#include "visual/connectedglass.hpp"
#include "player/skinstealer.hpp"
#include "player/autogg.hpp"
#include "player/autoreq.hpp"
#include "misc/hiveutils.hpp"
#include "visual/swingmodifier.hpp"

namespace bedrocktc {


bool ModuleRegistry::registerModule(std::unique_ptr<Module> module) {
    if (!module || mById.find(module->moduleId) != mById.end()) return false;
    auto* raw=module.get(); mById.emplace(raw->moduleId,raw); mView.push_back(raw); mOwned.push_back(std::move(module)); return true;
}
bool ModuleRegistry::unregisterModule(std::string_view id) {
    auto it=mById.find(id); if(it==mById.end()) return false; Module* victim=it->second;
    if(victim->enabled) victim->disable();
    mView.erase(std::remove(mView.begin(),mView.end(),victim),mView.end()); mById.erase(it);
    mOwned.erase(std::remove_if(mOwned.begin(),mOwned.end(),[victim](auto& p){return p.get()==victim;}),mOwned.end()); return true;
}

ModuleRegistry& ModuleRegistry::get() {
    static ModuleRegistry registry;
    return registry;
}

Module* ModuleRegistry::find(std::string_view id) const {
    const auto it = mById.find(id);
    return it == mById.end() ? nullptr : it->second;
}

const std::vector<Module*>& ModuleRegistry::modules() const {
    return mView;
}

void ModuleRegistry::shutdown() { for(auto* m:mView) if(m->enabled) m->disable(); mInitialized=false; }

void ModuleRegistry::initialize() {
    if (mInitialized) return;

    BTP_LOGI("ModuleRegistry: initializing %zu modules", mView.size());
    std::size_t failed = 0;
    for (auto* module : mView) {
        if (!module) {
            ++failed;
            BTP_LOGE("ModuleRegistry: null module entry");
            continue;
        }

        BTP_LOGI("ModuleRegistry: onInit begin %s", module->moduleId.c_str());
        try {
            module->onInit();
            BTP_LOGI("ModuleRegistry: onInit complete %s", module->moduleId.c_str());
        } catch (const std::exception& e) {
            ++failed;
            BTP_LOGE("ModuleRegistry: onInit failed %s: %s", module->moduleId.c_str(), e.what());
        } catch (...) {
            ++failed;
            BTP_LOGE("ModuleRegistry: onInit failed %s: unknown exception", module->moduleId.c_str());
        }
    }

    mInitialized = true;
    BTP_LOGI("ModuleRegistry: initialization complete, failed=%zu", failed);
}

void ModuleRegistry::onFrame() {
    for (auto* module : mView) if (module->enabled) module->onFrame();
}

bool ModuleRegistry::onMouseEvent(int button, bool isDown) {
    bool consumed = false;
    for (auto* module : mView) if (module->onMouseEvent(button, isDown)) consumed = true;
    return consumed;
}

void ModuleRegistry::setKeybindBlocked(bool blocked) {
    mKeybindBlocked = blocked;
}

bool ModuleRegistry::keybindBlocked() const {
    return mKeybindBlocked;
}


void registerAllModules() {
    auto& registry = ModuleRegistry::get();
    if (!registry.modules().empty()) return;
    registry.emplace<FullbrightModule>();
    registry.emplace<MotionBlurModule>();
    registry.emplace<FogColorModule>();
    registry.emplace<GlintColorModule>();
    registry.emplace<TntTimerModule>();
    registry.emplace<NoFogModule>();
    registry.emplace<PingCounterModule>();
    registry.emplace<ReachCounterModule>();
    registry.emplace<ComboDisplay>();
    registry.emplace<BreakIndicatorModule>();
    registry.emplace<PlayerCoordsModule>();
    registry.emplace<CompassModule>();
    registry.emplace<ArmorHudModule>();
    registry.emplace<PotionHudModule>();
    registry.emplace<TimeChangerModule>();
    registry.emplace<WorldTimeModule>();
    registry.emplace<TotemCounterModule>();
    registry.emplace<ArrowCounterModule>();
    registry.emplace<PotCounterModule>();
    registry.emplace<QuickLootModule>();
    registry.emplace<AutoSprintModule>();
    registry.emplace<WeatherChangerModule>();
    registry.emplace<NickModule>();
    registry.emplace<NoDisconnectModule>();
    registry.emplace<ChatTimestampsModule>();
    registry.emplace<NoTouchBorderModule>();
    registry.emplace<CpsLimiterModule>();
    registry.emplace<SpeedDisplayModule>();
    registry.emplace<DebugMenuModule>();
    registry.emplace<ViewModelModule>();
    registry.emplace<SwingModifierModule>();
    registry.emplace<KeystrokesModule>();
    registry.emplace<ThirdPersonNametagModule>();
    registry.emplace<TablistModule>();
    registry.emplace<ChunkBorderModule>();
    registry.emplace<HitboxModule>();
    registry.emplace<ZoomModule>();
    registry.emplace<CinematicCameraModule>();
    registry.emplace<BreadcrumbsModule>();
    registry.emplace<SkinStealerModule>();
    registry.emplace<AutoGG>();
    registry.emplace<AutoReQ>();
    registry.emplace<HiveUtilsModule>();
    registry.emplace<FPSUnlockerModule>();
    registry.emplace<LightOverlayModule>();
    registry.emplace<ShulkerPreviewModule>();
    registry.emplace<ConnectedGlassModule>();
    registry.emplace<ForceGlobalRPModule>();
}

} // namespace bedrocktc
