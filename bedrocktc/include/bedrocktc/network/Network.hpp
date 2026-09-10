#pragma once
#include <bedrocktc/client/Client.hpp>
#include <bedrocktc/sdk/network/Packet.hpp>
namespace bedrocktc::network {
using Packet=sdk::Packet; using TextPacket=sdk::TextPacket;
inline void* packetSender(){ return client::getPacketSender(); }
}
