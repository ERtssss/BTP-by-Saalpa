#pragma once
#include <bedrocktc/events/EventBus.hpp>
#include <string>
#include <string_view>

namespace btp::gui {
class Gui {
public:
    static Gui& get();
    void initialize();
    void shutdown();
    void draw();
    void toggle();
    bool visible() const noexcept;
    void selectCategory(int index);
    int category() const noexcept;
    void setSearch(std::string query);
    const std::string& search() const noexcept;
    void selectModule(std::string id);
    const std::string& selectedModule() const noexcept;
private:
    Gui() = default;
    void drawHeader();
    void drawSidebar();
    void drawModules();
    void drawSettings();
    bool handleTouch(float x, float y);
    bool mVisible = false;
    bool mInitialized = false;
    int mCategory = 0;
    std::string mSearch;
    std::string mSelectedModule;
    bedrocktc::events::Subscription mFrameSubscription = 0;
    bedrocktc::events::Subscription mTouchSubscription = 0;
    bedrocktc::events::Subscription mKeySubscription = 0;
};
}
