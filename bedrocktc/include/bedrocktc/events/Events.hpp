#pragma once
#include <bedrocktc/events/Event.hpp>
#include <bedrocktc/events/FrameEvent.hpp>
#include <bedrocktc/events/LocalPlayerTickEvent.hpp>
#include <bedrocktc/events/LocalPlayerPreTickEvent.hpp>
#include <bedrocktc/events/ClientInstanceUpdateEvent.hpp>
#include <bedrocktc/events/AttackEvent.hpp>
#include <bedrocktc/events/GameModeActionEvent.hpp>
#include <bedrocktc/events/MouseInputEvent.hpp>
#include <bedrocktc/events/ScreenStateEvent.hpp>
#include <bedrocktc/events/ContainerSlotSelectedEvent.hpp>

namespace bedrocktc::events {
struct TouchInputEvent {
    static constexpr EventType type = EventType::TouchInput;
    int action{};
    int pointerId{};
    float x{};
    float y{};
    bool cancelledValue{};
    void cancel() noexcept { cancelledValue = true; }
    bool cancelled() const noexcept { return cancelledValue; }
};
struct KeyInputEvent {
    static constexpr EventType type = EventType::KeyInput;
    int keyCode{};
    unsigned int unicodeChar{};
    bool isKeyDown{};
    bool cancelledValue{};
    void cancel() noexcept { cancelledValue = true; }
    bool cancelled() const noexcept { return cancelledValue; }
};
}
