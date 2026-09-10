#pragma once

#include <bedrocktc/sdk/Memory.hpp>
#include <bedrocktc/sdk/Offsets.hpp>
#include <bedrocktc/sdk/render/LevelRendererPlayer.hpp>

namespace bedrocktc::sdk {

class LevelRenderer {
public:
    void* renderChunkCoordinatorTable() {
        return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(this) + offsets::LevelRenderer::mRenderChunkCoordinators);
    }

    LevelRendererPlayer* playerRenderer() { return field<LevelRendererPlayer*>(this, offsets::LevelRenderer::mLevelRendererPlayer); }
};

}
