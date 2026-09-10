#include <bedrocktc/Api.hpp>
#include <bedrocktc/events/EventBus.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/hooks/Hooks.hpp>
#include "hooks/GameHooks.hpp"
namespace {
using namespace bedrocktc;
std::uintptr_t resolveSignature(std::uint16_t id){
 if(id>=static_cast<std::uint16_t>(memory::SignatureId::Count)) return 0;
 return memory::resolve(static_cast<memory::SignatureId>(id));
}
sdk::ClientInstance* clientInstance(){return reinterpret_cast<sdk::ClientInstance*>(core::gamehooks::clientInstance());}
std::uint64_t subscribe(events::EventType t,events::EventPriority p,api::EventCallback cb,void* u){
 if(!cb)return 0;
 return events::bus().subscribeRaw(t,[=](void* payload){cb(t,payload,u);},p);
}
void unsubscribe(std::uint64_t id){events::bus().unsubscribe(id);}
const api::ApiV1 gApi{api::AbiVersion,sizeof(api::ApiV1),resolveSignature,clientInstance,subscribe,unsubscribe};
}
namespace bedrocktc::api { const ApiV1* getApi(std::uint32_t v){return v==AbiVersion?&gApi:nullptr;} }
extern "C" BEDROCKTC_API const bedrocktc::api::ApiV1* BedrockTC_GetApi(std::uint32_t v){return bedrocktc::api::getApi(v);}
