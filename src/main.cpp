#include <btp/core/Runtime.hpp>
#include <bedrocktc/Core.hpp>
#include <pl/Mod.hpp>

class BTPMod {
public:
    static BTPMod& instance() { static BTPMod value; return value; }
    BTPMod() : mSelf(*ll::mod::NativeMod::current()) {}

    ll::mod::NativeMod& getSelf() const noexcept { return mSelf; }

    bool load() noexcept {
        try {
            return btp::core::Runtime::get().load(mSelf);
        } catch (...) {
            mSelf.getLogger().error("BTP load() failed");
            return false;
        }
    }

    bool enable() noexcept {
        try {
            return btp::core::Runtime::get().enable();
        } catch (...) {
            mSelf.getLogger().error("BTP enable() failed");
            return false;
        }
    }

    bool disable() noexcept {
        try {
            return btp::core::Runtime::get().disable();
        } catch (...) {
            mSelf.getLogger().error("BTP disable() failed");
            return false;
        }
    }

    bool unload() noexcept {
        try {
            return btp::core::Runtime::get().unload();
        } catch (...) {
            mSelf.getLogger().error("BTP unload() failed");
            return false;
        }
    }

private:
    ll::mod::NativeMod& mSelf;
};

PL_REGISTER_MOD(BTPMod, BTPMod::instance())
