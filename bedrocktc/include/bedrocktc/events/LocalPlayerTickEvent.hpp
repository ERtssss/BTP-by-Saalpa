#pragma once

#include <bedrocktc/events/Event.hpp>

namespace bedrocktc::sdk { class Player; }

namespace bedrocktc::events {

struct LocalPlayerTickEvent {
    static constexpr EventType type = EventType::LocalPlayerTick;
    sdk::Player* player;
};

}
