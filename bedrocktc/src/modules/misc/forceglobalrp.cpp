#include "forceglobalrp.hpp"
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/network/Packet.hpp>
#include <bedrocktc/hooks/Hooks.hpp>

namespace bedrocktc {


static ForceGlobalRPModule* g_forceGlobalRPMod = nullptr;

static void (*original_ResourcePackStackPacket_handle)(void* _this, void* a1, void* a2, std::shared_ptr<::bedrocktc::sdk::Packet>& packet) = nullptr;
static void (*original_ResourcePacksInfoPacket_handle)(void* _this, void* a1, void* a2, std::shared_ptr<::bedrocktc::sdk::Packet>& packet) = nullptr;
static void ResourcePacksInfoPacket_handle_hook(void* _this, void* a1, void* a2, std::shared_ptr<::bedrocktc::sdk::Packet>& packet) {
    if (g_forceGlobalRPMod && g_forceGlobalRPMod->enabled && packet.get() != nullptr) {
        ::bedrocktc::sdk::Packet* pkt = packet.get();
        ::bedrocktc::sdk::field<bool>(pkt, ::bedrocktc::sdk::offsets::ResourcePacksInfoPacket::mResourcePackRequired) = false;
        ::bedrocktc::sdk::field<bool>(pkt, ::bedrocktc::sdk::offsets::ResourcePacksInfoPacket::mForceDisableVibrantVisuals) = false;
    }
    
    if (original_ResourcePacksInfoPacket_handle) {
        original_ResourcePacksInfoPacket_handle(_this, a1, a2, packet);
    }
}
static void ResourcePackStackPacket_handle_hook(void* _this, void* a1, void* a2, std::shared_ptr<::bedrocktc::sdk::Packet>& packet) {
    if (g_forceGlobalRPMod && g_forceGlobalRPMod->enabled && packet.get() != nullptr) {
        ::bedrocktc::sdk::Packet* pkt = packet.get();
        ::bedrocktc::sdk::field<bool>(pkt, ::bedrocktc::sdk::offsets::ResourcePackStackPacket::mResourcePackRequired) = false;
    }
    
    if (original_ResourcePacksInfoPacket_handle) {
        original_ResourcePackStackPacket_handle(_this, a1, a2, packet);
    }
}

void ForceGlobalRPModule::initFunc1() {
    if (m_func1Target) return;
    
    uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::ResourcePacksInfoPacketHandle);
    if (addr != 0) {
        m_func1Target = (void*)addr;
        ::bedrocktc::hooks::install(m_func1Target, (void*)ResourcePacksInfoPacket_handle_hook, (void**)&original_ResourcePacksInfoPacket_handle);
        m_func1_hooked = true;
    }
}

void ForceGlobalRPModule::initFunc2() {
    if (m_func2Target) return;
    
    uintptr_t addr = ::bedrocktc::memory::resolve(::bedrocktc::memory::SignatureId::ResourcePackStackPacketHandle);
    if (addr != 0) {
        m_func2Target = (void*)addr;
        ::bedrocktc::hooks::install(m_func2Target, (void*)ResourcePackStackPacket_handle_hook, (void**)&original_ResourcePackStackPacket_handle);
        m_func2_hooked = true;
    }
}

ForceGlobalRPModule::ForceGlobalRPModule()
    : Module("ForceGlobalRP", "force to use global resource packs if the server has the `Required Packs` Option enabled & forcing the use of Vibrant Visuals on a server that forces it to be turned off.") {
    g_forceGlobalRPMod = this;
}

ForceGlobalRPModule::~ForceGlobalRPModule() {
    if (g_forceGlobalRPMod == this) g_forceGlobalRPMod = nullptr;
}

void ForceGlobalRPModule::onInit() {
    initFunc1();
    initFunc2();
}

void ForceGlobalRPModule::onEnable() {
}

void ForceGlobalRPModule::onDisable() {
}


} // namespace bedrocktc
