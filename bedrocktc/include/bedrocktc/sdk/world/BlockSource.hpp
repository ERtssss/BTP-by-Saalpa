#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>

namespace bedrocktc::sdk {

class BlockSource {
public:
    int dimensionId() {
        return virtualCall<int>(this, offsets::VTable::BlockSource_getDimensionId);
    }
};

}
