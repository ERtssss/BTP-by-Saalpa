#include <bedrocktc/events/EventBus.hpp>

namespace bedrocktc::events {

EventBus& bus() {
    static EventBus instance;
    return instance;
}

}
