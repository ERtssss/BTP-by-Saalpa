#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class NoDisconnectModule : public ::bedrocktc::Module {
public:
    NoDisconnectModule();
    void onInit() override;
    void onEnable() override;
    void onDisable() override;

private:
    bool m_patched = false;
    void* m_patchTarget = nullptr;
};

} // namespace bedrocktc
