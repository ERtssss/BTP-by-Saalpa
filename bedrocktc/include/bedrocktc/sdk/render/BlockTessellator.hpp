#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/world/BlockSource.hpp>

namespace bedrocktc::sdk {

class BlockTessellator {
public:
    BlockSource* region() { return field<BlockSource*>(this, offsets::BlockTessellator::mRegion); }
    bool usesInternalTexture() const { return field<std::uint8_t>(this, offsets::BlockTessellator::mUseInternalTexture) != 0; }
    std::uint8_t& xFlipTexture() { return field<std::uint8_t>(this, offsets::BlockTessellator::mXFlipTexture); }
    void* internalTexture() { return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(this) + offsets::BlockTessellator::mInternalTexture); }
};

}
