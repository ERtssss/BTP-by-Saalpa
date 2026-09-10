#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class NoFogModule : public ::bedrocktc::Module {
public:
    NoFogModule();
    ~NoFogModule() override;

    void onInit() override;
    void onEnable() override;
    void onDisable() override;

private:
    bool m_patched;
    void* m_patchTarget;

    void applyPatch();
};

} // namespace bedrocktc
