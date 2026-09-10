#pragma once
#include <bedrocktc/Export.hpp>
#include <bedrocktc/events/Events.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <cstddef>
#include <cstdint>
namespace bedrocktc::sdk { class ClientInstance; }
namespace bedrocktc::api {
inline constexpr std::uint32_t AbiVersion = 1;
using EventCallback = void(*)(events::EventType type, void* payload, void* userData);
struct ApiV1 {
 std::uint32_t abiVersion;
 std::uint32_t structSize;
 std::uintptr_t (*resolveSignature)(std::uint16_t id);
 sdk::ClientInstance* (*clientInstance)();
 std::uint64_t (*subscribe)(events::EventType, events::EventPriority, EventCallback, void*);
 void (*unsubscribe)(std::uint64_t);
};
BEDROCKTC_API const ApiV1* getApi(std::uint32_t version = AbiVersion);

inline bool compatible(const ApiV1* api) noexcept {
    return api && api->abiVersion == AbiVersion &&
           api->structSize >= sizeof(ApiV1);
}

inline std::uintptr_t resolve(memory::SignatureId id, const ApiV1* api = nullptr) noexcept {
    if (!api) api = getApi();
    if (!compatible(api) || !api->resolveSignature) return 0;
    return api->resolveSignature(static_cast<std::uint16_t>(id));
}
}
extern "C" BEDROCKTC_API const bedrocktc::api::ApiV1* BedrockTC_GetApi(std::uint32_t version);
