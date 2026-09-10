#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <cstdint>
#include <cstring>
#include <bedrocktc/patches/Patch.hpp>

namespace bedrocktc {



class FullbrightModule : public ::bedrocktc::Module {
public:
    FullbrightModule();
    void onInit() override;
    void onEnable() override;
    void onDisable() override;

private:
    bool    m_patched = false;
    ::bedrocktc::patches::Handle m_patchHandle;
    void*   m_patchTarget = nullptr;
};

} // namespace bedrocktc
