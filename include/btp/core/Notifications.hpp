#pragma once
#include <bedrocktc/events/EventBus.hpp>
#include <deque>
#include <string>

namespace btp::core {
enum class NotificationType { Info, Success, Warning, Error };
struct Notification { NotificationType type; std::string text; float ttl; };
class Notifications {
public:
    static Notifications& get();
    void push(NotificationType type, std::string text, float seconds = 2.5f);
    void update(float dt);
    void draw();
private:
    std::deque<Notification> mItems;
};
}
