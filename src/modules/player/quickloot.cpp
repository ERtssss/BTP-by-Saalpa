#include "quickloot.hpp"

#include <bedrocktoolsplus/events/EventBus.hpp>
#include <bedrocktoolsplus/memory/Signatures.hpp>
#include <limits>

QuickLootModule::QuickLootModule()
    : Module("Quick Loot", "Moves a tapped item stack instantly between a container and your inventory.") {}

void QuickLootModule::onInit() {
    m_handleAutoPlace = reinterpret_cast<HandleAutoPlaceFn>(
        bedrocktoolsplus::memory::resolve(bedrocktoolsplus::memory::SignatureId::ContainerScreenControllerHandleAutoPlace)
    );

    bedrocktoolsplus::events::bus().subscribe<bedrocktoolsplus::events::ContainerSlotSelectedEvent>(
        [this](auto& event) {
            if (event.afterSelection || !enabled || !m_handleAutoPlace || m_transferring || !event.controller ||
                event.index < 0 || event.collectionName.empty()) return;
            m_transferring = true;
            struct TransferGuard {
                bool& active;
                ~TransferGuard() { active = false; }
            } guard{m_transferring};
            m_handleAutoPlace(event.controller, std::numeric_limits<int>::max(), event.collectionName, event.index);
            event.cancel();
        },
        bedrocktoolsplus::events::EventPriority::First
    );
}
