#pragma once

#include <bedrocktc/events/Event.hpp>

namespace bedrocktc::events {

struct FrameEvent {
    static constexpr EventType type = EventType::Frame;
};

}
