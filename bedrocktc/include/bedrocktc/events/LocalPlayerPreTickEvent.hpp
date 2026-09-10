#pragma once

#include <bedrocktc/events/Event.hpp>

namespace bedrocktc::sdk { class Player; }

namespace bedrocktc::events {

struct LocalPlayerPreTickEvent {
    static constexpr EventType type = EventType::LocalPlayerPreTick;
    sdk::Player* player;
};

}
