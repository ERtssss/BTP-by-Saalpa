#pragma once

#include <bedrocktc/events/Event.hpp>
#include <string>

namespace bedrocktc::events {

struct ContainerSlotSelectedEvent : Cancellable {
    static constexpr EventType type = EventType::ContainerSlotSelected;

    ContainerSlotSelectedEvent(void* screenController, const std::string& collection, int slot)
        : controller(screenController), collectionName(collection), index(slot) {}

    void* controller;
    const std::string& collectionName;
    int index;
    bool afterSelection = false;
};

}
