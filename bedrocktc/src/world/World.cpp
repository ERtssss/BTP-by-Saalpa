#include <bedrocktc/world/World.hpp>
#include <bedrocktc/memory/Signatures.hpp>
namespace bedrocktc::world {
Level* getLevel() {
 auto* p=client::getLocalPlayer();
 return p ? p->level() : nullptr;
}
std::vector<Entity*> getEntities() {
 std::vector<Entity*> out;
 auto* level=getLevel(); if(!level) return out;
 void* manager=level->actorManager(); if(!manager) return out;
 using ActorListFn=std::vector<void*>(*)(void*);
 auto fn=reinterpret_cast<ActorListFn>(memory::resolve(memory::SignatureId::ActorManagerList));
 if(!fn) return out;
 auto raw=fn(manager); out.reserve(raw.size());
 for(void* e:raw) if(e) out.push_back(reinterpret_cast<Entity*>(e));
 return out;
}
}
