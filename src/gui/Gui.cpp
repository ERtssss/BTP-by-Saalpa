#include <btp/gui/Gui.hpp>
#include <btp/core/ModuleManager.hpp>
#include <btp/debug/Log.hpp>
#include <bedrocktc/render/Render.hpp>
#include <bedrocktc/events/Events.hpp>
#include <bedrocktc/sdk/client/ClientInstance.hpp>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace btp::gui {
namespace {
constexpr float BASE_W = 1156.0f;
constexpr float BASE_H = 704.0f;
constexpr float PANEL_X = 90.0f;
constexpr float PANEL_Y = 42.0f;
constexpr float PANEL_W = 976.0f;
constexpr float PANEL_H = 620.0f;
constexpr float SIDEBAR_W = 190.0f;
constexpr float ROW_H = 54.0f;
constexpr std::uint32_t BG = 0xF0181A1F;
constexpr std::uint32_t PANEL = 0xF022252B;
constexpr std::uint32_t SIDEBAR = 0xF01B1E23;
constexpr std::uint32_t CARD = 0xF02A2E35;
constexpr std::uint32_t CARD_ON = 0xF035414B;
constexpr std::uint32_t TEXT = 0xFFFFFFFF;
constexpr std::uint32_t MUTED = 0xFFB0B5BD;
constexpr std::uint32_t ACCENT = 0xFF65B5FF;
constexpr std::uint32_t OFF = 0xFF777D86;

const char* categoryName(core::Category c) {
    switch (c) {
        case core::Category::Combat: return "Combat";
        case core::Category::Visuals: return "Visuals";
        case core::Category::Player: return "Player";
        case core::Category::Movement: return "Movement";
        case core::Category::World: return "World";
        case core::Category::HUD: return "HUD";
        case core::Category::Render: return "Render";
        case core::Category::Utility: return "Utility";
        case core::Category::Misc: return "Misc";
        case core::Category::Settings: return "All";
    }
    return "All";
}

bool inside(float x, float y, float rx, float ry, float rw, float rh) {
    return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

bool inGame() {
    auto* client = bedrocktc::sdk::ClientInstance::current();
    return client && client->localPlayer() != nullptr;
}
}

Gui& Gui::get() { static Gui instance; return instance; }

void Gui::initialize() {
    if (mInitialized) return;
    mInitialized = true;
    mVisible = false;
    mCategory = static_cast<int>(core::Category::Settings);

    auto& bus = bedrocktc::events::bus();
    mTouchSubscription = bus.subscribe<bedrocktc::events::TouchInputEvent>([](auto& e) {
        // ACTION_UP (1) prevents a drag/move from repeatedly toggling modules.
        if (e.action != 1) return;
        Gui::get().handleTouch(e.x, e.y);
        if (Gui::get().visible()) e.cancel();
    }, bedrocktc::events::EventPriority::Early);

    mKeySubscription = bus.subscribe<bedrocktc::events::KeyInputEvent>([](auto& e) {
        // Android KEYCODE_ESCAPE / BACK and F8 are convenient ways to open/close
        // the menu without adding a launcher dependency.
        if (!e.isKeyDown) return;
        if (e.keyCode == 111 || e.keyCode == 4 || e.keyCode == 142) {
            Gui::get().toggle();
            e.cancel();
        }
    }, bedrocktc::events::EventPriority::Early);

    BTP_LOGI("In-game GUI initialized");
    bedrocktc::render::native::setOverlayCallback([] {
        try { Gui::get().draw(); }
        catch (const std::exception& e) { BTP_LOGE("GUI draw exception: %s", e.what()); }
        catch (...) { BTP_LOGE("GUI draw unknown exception"); }
    });
}

void Gui::shutdown() {
    if (!mInitialized) return;
    auto& bus = bedrocktc::events::bus();
    if (mTouchSubscription) bus.unsubscribe(mTouchSubscription);
    if (mKeySubscription) bus.unsubscribe(mKeySubscription);
    mTouchSubscription = 0;
    mKeySubscription = 0;
    bedrocktc::render::native::setOverlayCallback(nullptr);
    mInitialized = false;
    mVisible = false;
}

void Gui::drawHeader() {
    bedrocktc::render::native::fillRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, PANEL);
    bedrocktc::render::native::fillRect(PANEL_X, PANEL_Y, PANEL_W, 64, CARD);
    bedrocktc::render::native::text(PANEL_X + 24, PANEL_Y + 13, 400, 36, 26, "BTP", TEXT, true);
    bedrocktc::render::native::text(PANEL_X + 96, PANEL_Y + 17, 420, 30, 18, "Bedrock Tools Plus", MUTED, false);
    bedrocktc::render::native::text(PANEL_X + PANEL_W - 95, PANEL_Y + 17, 70, 30, 17, "CLOSE", MUTED, false);
}

void Gui::drawSidebar() {
    bedrocktc::render::native::fillRect(PANEL_X, PANEL_Y + 64, SIDEBAR_W, PANEL_H - 64, SIDEBAR);
    constexpr core::Category categories[] = {
        core::Category::Settings, core::Category::Combat, core::Category::Visuals,
        core::Category::Player, core::Category::Movement, core::Category::World,
        core::Category::HUD, core::Category::Render, core::Category::Utility,
        core::Category::Misc};
    float y = PANEL_Y + 82;
    for (auto c : categories) {
        const bool selected = mCategory == static_cast<int>(c);
        if (selected) bedrocktc::render::native::fillRect(PANEL_X + 10, y - 5, SIDEBAR_W - 20, 42, CARD_ON);
        bedrocktc::render::native::text(PANEL_X + 26, y + 3, SIDEBAR_W - 45, 28, 17,
                                         categoryName(c), selected ? TEXT : MUTED, false);
        y += 48;
    }
}

void Gui::drawModules() {
    const auto category = static_cast<core::Category>(std::clamp(mCategory, 0, 9));
    auto modules = core::ModuleManager::get().search(mSearch, category);
    const float x = PANEL_X + SIDEBAR_W + 22;
    const float y0 = PANEL_Y + 84;
    const float w = PANEL_W - SIDEBAR_W - 44;

    bedrocktc::render::native::text(x, PANEL_Y + 77, w, 32, 22,
                                    mSearch.empty() ? "Modules" : mSearch, TEXT, false);

    std::size_t visible = 0;
    for (const auto& view : modules) {
        if (!view.module || visible >= 9) break;
        const float y = y0 + static_cast<float>(visible) * ROW_H;
        const bool on = view.module->isEnabled();
        bedrocktc::render::native::fillRect(x, y, w, ROW_H - 7, on ? CARD_ON : CARD);
        bedrocktc::render::native::text(x + 16, y + 7, w - 120, 25, 18, view.module->name, TEXT, false);
        bedrocktc::render::native::text(x + 16, y + 29, w - 145, 19, 12,
                                        view.module->description ? view.module->description : "", MUTED, false);
        bedrocktc::render::native::text(x + w - 82, y + 15, 60, 25, 15, on ? "ON" : "OFF", on ? ACCENT : OFF, false);
        ++visible;
    }

    if (modules.empty()) {
        bedrocktc::render::native::text(x + 20, y0 + 20, w - 40, 30, 18, "No modules in this category", MUTED, false);
    }
}

void Gui::drawSettings() {
    if (mSelectedModule.empty()) return;
    auto* module = core::ModuleManager::get().find(mSelectedModule);
    if (!module) return;
    const float x = PANEL_X + SIDEBAR_W + 22;
    const float y = PANEL_Y + 574;
    std::string title = std::string("Selected: ") + module->name;
    bedrocktc::render::native::text(x, y, PANEL_W - SIDEBAR_W - 44, 26, 15, title, MUTED, false);
}

void Gui::draw() {
    if (!mInitialized) return;
    const auto screen = bedrocktc::render::native::size();
    if (screen.width <= 0 || screen.height <= 0) return;

    // Never render the BTP overlay on the title/login/launcher screens.
    // ClientInstance::localPlayer() is the game-state gate.
    // The render context is a stronger and safer gate than localPlayer().
    // ClientInstance::localPlayer() may still be null while the in-game UI
    // render context is already valid, which previously made the menu invisible.
    if (!bedrocktc::render::native::ready()) return;

    if (!mVisible) {
        bedrocktc::render::native::fillRect(12, 12, 110, 48, CARD);
        bedrocktc::render::native::fillRect(12, 12, 4, 48, ACCENT);
        bedrocktc::render::native::text(28, 23, 80, 25, 18, "BTP", TEXT, true);
        bedrocktc::render::native::flushText();
        return;
    }

    drawHeader();
    drawSidebar();
    drawModules();
    drawSettings();
    bedrocktc::render::native::flushText();
}

void Gui::toggle() {
    if (!mInitialized) return;
    mVisible = !mVisible;
    BTP_LOGI("In-game GUI %s", mVisible ? "opened" : "closed");
}

bool Gui::visible() const noexcept { return mVisible; }

void Gui::selectCategory(int index) {
    mCategory = std::clamp(index, 0, 9);
    mSelectedModule.clear();
}

int Gui::category() const noexcept { return mCategory; }
void Gui::setSearch(std::string query) { mSearch = std::move(query); }
const std::string& Gui::search() const noexcept { return mSearch; }
void Gui::selectModule(std::string id) { mSelectedModule = std::move(id); }
const std::string& Gui::selectedModule() const noexcept { return mSelectedModule; }

bool Gui::handleTouch(float x, float y) {
    if (!mInitialized || !bedrocktc::render::native::ready()) return false;
    const auto screen = bedrocktc::render::native::size();
    if (screen.width <= 0 || screen.height <= 0) return false;
    const float sx = BASE_W / screen.width;
    const float sy = BASE_H / screen.height;
    x *= sx;
    y *= sy;

    // Small BTP hotspot in the upper-left while the menu is closed.
    if (!mVisible) {
        if (inside(x, y, 12, 12, 110, 48)) {
            toggle();
            return true;
        }
        return false;
    }

    if (inside(x, y, PANEL_X + PANEL_W - 120, PANEL_Y, 120, 64)) {
        toggle();
        return true;
    }

    constexpr core::Category categories[] = {
        core::Category::Settings, core::Category::Combat, core::Category::Visuals,
        core::Category::Player, core::Category::Movement, core::Category::World,
        core::Category::HUD, core::Category::Render, core::Category::Utility,
        core::Category::Misc};
    float cy = PANEL_Y + 77;
    for (int i = 0; i < 10; ++i) {
        if (inside(x, y, PANEL_X, cy - 5, SIDEBAR_W, 42)) {
            selectCategory(static_cast<int>(categories[i]));
            return true;
        }
        cy += 48;
    }

    const auto category = static_cast<core::Category>(std::clamp(mCategory, 0, 9));
    auto modules = core::ModuleManager::get().search(mSearch, category);
    const float rx = PANEL_X + SIDEBAR_W + 22;
    const float ry = PANEL_Y + 84;
    const float rw = PANEL_W - SIDEBAR_W - 44;
    for (std::size_t i = 0; i < modules.size() && i < 9; ++i) {
        const float rowY = ry + static_cast<float>(i) * ROW_H;
        if (inside(x, y, rx, rowY, rw, ROW_H - 7) && modules[i].module) {
            auto* module = modules[i].module;
            module->toggle();
            selectModule(module->moduleId);
            BTP_LOGI("GUI toggled module %s -> %s", module->moduleId.c_str(), module->isEnabled() ? "ON" : "OFF");
            return true;
        }
    }
    return true;
}
}
