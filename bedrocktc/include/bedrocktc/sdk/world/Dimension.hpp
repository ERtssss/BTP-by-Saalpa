#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/world/BlockSource.hpp>
#include <bedrocktc/sdk/world/Weather.hpp>

namespace bedrocktc::sdk {

class Dimension {
public:
    BlockSource* blockSource() { return field<BlockSource*>(this, offsets::Dimension::mBlockSource); }
    Weather* weather() { return field<Weather*>(this, offsets::Dimension::mWeather); }
};

}
