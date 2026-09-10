#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/Types.hpp>
#include <bedrocktc/sdk/world/Dimension.hpp>
#include <bedrocktc/sdk/world/Level.hpp>
#include <cstdint>
#include <string>

namespace bedrocktc::sdk {

class Actor {
public:
    void* entityContext() {
        return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(this) + offsets::Actor::mEntityContext);
    }

    void* stateVectorComponent() const { return field<void*>(this, offsets::Actor::mStateVectorComponent); }
    void* rotationComponent() const { return field<void*>(this, offsets::Actor::mActorRotationComponent); }
    Level* level() const { return field<Level*>(this, offsets::Actor::mLevel); }
    Dimension* dimension() const { return field<Dimension*>(this, offsets::Actor::mDimension); }
    int hurtTime() const { return field<int>(this, offsets::Actor::mHurtTime); }
    std::uint32_t categories() const { return field<std::uint32_t>(this, offsets::Actor::mCategories); }

    Vec3 position() const {
        auto* component = stateVectorComponent();
        return component ? field<Vec3>(component, 0) : Vec3{};
    }

    Vec2 rotation() const {
        auto* component = rotationComponent();
        return component ? field<Vec2>(component, 0) : Vec2{};
    }

    AABB bounds() const {
        auto* component = field<void*>(this, offsets::Actor::mStateVectorComponent + offsets::BuiltInActorComponents::mAABBShapeComponent);
        if (!component) return {};
        return field<AABB>(component, offsets::AABBShapeComponent::mAABB);
    }

    bool hasCategory(std::uint32_t category) const { return (categories() & category) != 0; }
};

class Player : public Actor {
public:
    std::string& name() { return field<std::string>(this, offsets::Player::mName); }
    const std::string& name() const { return field<std::string>(this, offsets::Player::mName); }
    void* skin() const { return field<void*>(this, offsets::Player::mSkin); }
};

}
