#pragma once

#include <bedrocktc/events/Event.hpp>

namespace bedrocktc::events {

struct MouseInputEvent : Cancellable {
    static constexpr EventType type = EventType::MouseInput;

    MouseInputEvent(int mouseButton, bool isDown) : button(mouseButton), down(isDown) {}

    int button;
    bool down;
};

}
