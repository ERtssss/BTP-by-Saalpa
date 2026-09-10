#pragma once

#include <bedrocktc/Api.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/Functions.hpp>
#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/render/LevelRenderer.hpp>
#include <bedrocktc/sdk/world/Actor.hpp>
#include <bedrocktc/sdk/world/BlockSource.hpp>

namespace bedrocktc::sdk {

class ClientInstance {
public:
    static ClientInstance* current() {
        const auto* runtime = api::getApi();
        return api::compatible(runtime) && runtime->clientInstance ? runtime->clientInstance() : nullptr;
    }

    BlockSource* region() { return virtualCall<BlockSource*>(this, offsets::VTable::ClientInstance_getRegion); }
    void* minecraftGame() { return virtualCall<void*>(this, offsets::VTable::ClientInstanceGetMinecraftGame); }
    LevelRenderer* levelRenderer() { return field<LevelRenderer*>(this, offsets::ClientInstance::mLevelRenderer); }

    Player* localPlayer(const api::ApiV1* runtime = nullptr) {
        using Function = Player*(*)(ClientInstance*);
        auto target = function<Function>(memory::SignatureId::ClientInstanceGetLocalPlayer, runtime);
        return target ? target(this) : nullptr;
    }

    void* packetSender(const api::ApiV1* runtime = nullptr) {
        using Function = void*(*)(ClientInstance*);
        auto target = function<Function>(memory::SignatureId::ClientInstanceGetPacketSender, runtime);
        return target ? target(this) : nullptr;
    }
};

}
