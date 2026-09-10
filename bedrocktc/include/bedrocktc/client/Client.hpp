#pragma once
#include <bedrocktc/sdk/client/ClientInstance.hpp>
namespace bedrocktc::client {
using ClientInstance = sdk::ClientInstance;
using Player = sdk::Player;
inline ClientInstance* get() { return ClientInstance::current(); }
inline Player* getLocalPlayer() { auto* c=get(); return c ? c->localPlayer() : nullptr; }
inline void* getPacketSender() { auto* c=get(); return c ? c->packetSender() : nullptr; }
}
