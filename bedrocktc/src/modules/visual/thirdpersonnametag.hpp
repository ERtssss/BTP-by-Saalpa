#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <cstdint>
#include <bedrocktc/patches/Patch.hpp>

namespace bedrocktc {


class ThirdPersonNametagModule : public ::bedrocktc::Module {
private:
    bool m_patched;
    ::bedrocktc::patches::Handle m_patchHandle;
    void* m_patchTarget;

    void applyPatch();
    void removePatch();

public:
    ThirdPersonNametagModule();
    ~ThirdPersonNametagModule() override;

    void onInit() override;
    void onEnable() override;
    void onDisable() override;
};

} // namespace bedrocktc
