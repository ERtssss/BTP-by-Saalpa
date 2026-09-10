#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <bedrocktc/sdk/Types.hpp>

namespace bedrocktc {



class SpeedDisplayModule : public ::bedrocktc::Module {
public:
    SpeedDisplayModule();
    ~SpeedDisplayModule() override;
    
    void onInit() override;
    void onEnable() override;
    void onDisable() override;
    void onFrame() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;
    
    void updatePosition(const ::bedrocktc::sdk::Vec3& pos);

private:
    float m_currentSpeed = 0.0f;
    bool  m_use3D = false;
    
    ::bedrocktc::sdk::Vec3  m_lastPos;
    bool  m_firstTick = true;
    
    float hudPosX = 20.0f;
    float hudPosY = 100.0f;
    bool isHudModule = true;
    
    float m_size = 40.0f;
    bool m_background = false;
    float m_backgroundOpacity = 0.5f;
};

} // namespace bedrocktc
