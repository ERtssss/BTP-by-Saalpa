#pragma once
#include <bedrocktc/client/Client.hpp>
#include <bedrocktc/sdk/world/Actor.hpp>
#include <bedrocktc/sdk/world/Level.hpp>
#include <vector>
namespace bedrocktc::world {
using Entity = sdk::Actor;
using Player = sdk::Player;
using Level = sdk::Level;
Level* getLevel();
std::vector<Entity*> getEntities();
}
