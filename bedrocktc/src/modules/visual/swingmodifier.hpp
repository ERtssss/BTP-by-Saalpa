#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <cstdint>
#include <bedrocktc/patches/Patch.hpp>

namespace bedrocktc {


class SwingModifierModule : public ::bedrocktc::Module {
private:
    bool m_patched;
    ::bedrocktc::patches::Handle m_patchHandle1;
    ::bedrocktc::patches::Handle m_patchHandle2;
    void* m_patchTarget;
    void* m_patchTarget2;
    bool m_renderFirstPersonHooked;
    bool m_getModifiedSwingDurationHooked;

    
    
    public:
    int m_swingSpeed = 30;
    bool m_fluxSwing = false;
    
    SwingModifierModule();
    ~SwingModifierModule() override;
    
    void onInit() override;
    void onEnable() override;
    void onDisable() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;
    void applyPatch();
    void removePatch();
};

} // namespace bedrocktc
