#include <btp/core/Notifications.hpp>
#include <bedrocktc/render/Render.hpp>
#include <utility>

namespace btp::core {
Notifications& Notifications::get() { static Notifications instance; return instance; }
void Notifications::push(NotificationType type, std::string text, float seconds) {
    mItems.push_back({type, std::move(text), seconds});
    while (mItems.size() > 6) mItems.pop_front();
}
void Notifications::update(float dt) {
    for (auto& item : mItems) item.ttl -= dt;
    while (!mItems.empty() && mItems.front().ttl <= 0.f) mItems.pop_front();
}
void Notifications::draw() {
    float y = 24.f;
    for (const auto& item : mItems) {
        bedrocktc::render::rect("btp.notifications", 20, y, 300, 30, 0xCC101010, true);
        bedrocktc::render::text("btp.notifications", 32, y + 5, 276, 20, 14, item.text, 0xFFFFFFFF);
        y += 36.f;
    }
}
}
