#pragma once

#include <bedrocktc/events/Event.hpp>

namespace bedrocktc::sdk { class ClientInstance; }

namespace bedrocktc::events {

struct ClientInstanceUpdateEvent {
    static constexpr EventType type = EventType::ClientInstanceUpdate;
    sdk::ClientInstance* clientInstance;
};

}
