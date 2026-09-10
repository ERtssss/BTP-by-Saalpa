#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <string>

namespace bedrocktc::sdk {

class Block {
public:
    void* blockType() const { return field<void*>(this, offsets::Block::mBlockType); }

    const std::string* fullName() const {
        auto type = reinterpret_cast<std::uintptr_t>(blockType());
        if (!type) return nullptr;
        auto hashed = type + offsets::BlockType::mNameInfo + offsets::NameInfo::mFullName;
        return reinterpret_cast<const std::string*>(hashed + offsets::HashedString::mString);
    }
};

}
