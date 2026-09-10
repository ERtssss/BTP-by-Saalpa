#include "autosprint.hpp"
#include <bedrocktc/events/EventBus.hpp>
#include <bedrocktc/sdk/input/MoveInput.hpp>

namespace bedrocktc {


AutoSprintModule::AutoSprintModule()
    : Module("AutoSprint", "Holds Minecraft's native sprint input for you.") {}

void AutoSprintModule::onInit() {
    ::bedrocktc::events::bus().subscribe<::bedrocktc::events::LocalPlayerPreTickEvent>([this](auto& event) {
        if (!enabled || !event.player) return;
        auto* input = ::bedrocktc::sdk::moveInputComponent(event.player);
        if (!input) return;
        input->mRawInputState.set(MoveInputState::Flag::SprintDown, true);
    }, ::bedrocktc::events::EventPriority::First);
}

} // namespace bedrocktc
