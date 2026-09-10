#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/Types.hpp>

namespace bedrocktc::sdk {

class HitResult {
public:
    int type() const { return field<int>(this, offsets::HitResult::mType); }
    const Vec3& startPosition() const { return field<Vec3>(this, offsets::HitResult::mStartPos); }
    const Vec3& position() const { return field<Vec3>(this, offsets::HitResult::mPos); }
};

}
