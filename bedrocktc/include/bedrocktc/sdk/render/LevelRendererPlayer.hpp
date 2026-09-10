#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/Types.hpp>

namespace bedrocktc::sdk {

class LevelRendererPlayer {
public:
    float& fogColorRed() { return field<float>(this, offsets::LevelRendererPlayer::mFogColorRed); }
    float& fogColorGreen() { return field<float>(this, offsets::LevelRendererPlayer::mFogColorGreen); }
    float& fogColorBlue() { return field<float>(this, offsets::LevelRendererPlayer::mFogColorBlue); }
    float& baseFogStart() { return field<float>(this, offsets::LevelRendererPlayer::mBaseFogStart); }
    float& baseFogEnd() { return field<float>(this, offsets::LevelRendererPlayer::mBaseFogEnd); }
    float& currentFogDensityMax() { return field<float>(this, offsets::LevelRendererPlayer::mCurrentFogDensityMax); }
    Vec3& cameraPosition() { return field<Vec3>(this, offsets::LevelRendererPlayer::mCamPos); }
    void*& selectionOverlayMaterial() { return field<void*>(this, offsets::LevelRendererPlayer::mSelectionOverlayMaterial); }
};

}
