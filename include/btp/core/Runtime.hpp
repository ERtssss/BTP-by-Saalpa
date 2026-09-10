#pragma once
#include <filesystem>
#include <string>
#include <atomic>
#include <pl/Mod.hpp>
#include <bedrocktc/events/EventBus.hpp>

namespace btp::core {
class Runtime {
public:
    static Runtime& get();
    bool load(ll::mod::NativeMod& self);
    bool enable();
    bool disable();
    bool unload();
    bool active() const noexcept;
    const std::filesystem::path& configDir() const noexcept;
private:
    Runtime() = default;
    std::atomic_bool mActive{false};
    std::filesystem::path mConfigDir;
    std::string mOwnerModId;
    bedrocktc::events::Subscription mFrameSubscription = 0;
    bool mPostInitDone = false;
};
}
