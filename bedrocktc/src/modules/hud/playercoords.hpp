#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <bedrocktc/sdk/Types.hpp>

namespace bedrocktc {



class PlayerCoordsModule : public ::bedrocktc::Module {
public:
    PlayerCoordsModule();
    ~PlayerCoordsModule() override;
    
    void onInit() override;
    void onEnable() override;
    void onDisable() override;
    void onFrame() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;
    
    void updateCoords(const ::bedrocktc::sdk::Vec3& pos);

private:
    ::bedrocktc::sdk::Vec3  m_currentPos;
    float hudPosX = 20.0f;
    float hudPosY = 60.0f;
    bool isHudModule = true;
    
    float m_size = 40.0f;
    bool m_background = false;
    float m_backgroundOpacity = 0.5f;
};

} // namespace bedrocktc
