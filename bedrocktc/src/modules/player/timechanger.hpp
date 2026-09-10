#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class TimeChangerModule : public ::bedrocktc::Module {
public:
    TimeChangerModule();
    ~TimeChangerModule() override;
    
    void onInit() override;
    void onEnable() override;
    void onDisable() override;
    void onFrame() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

    int getCustomTime() const;
    int getRealTime(void* level);
    void updateRealTime(int newTime);

private:
    void applyPatch();

    int m_customTime = 18000;
    bool m_patched = false;
    void* m_patchTargetGetTime = nullptr;
    void* m_patchTargetSetTime = nullptr;
    int m_realTime = 0;
};

} // namespace bedrocktc
