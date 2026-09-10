#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <string>

namespace bedrocktc {



class SkinStealerModule : public ::bedrocktc::Module {
public:
    SkinStealerModule();
    ~SkinStealerModule() override;
    
    void onInit() override;
    void onEnable() override;
    void onDisable() override;
    void onFrame() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;
    
    void showStealMessage(const std::string& name);

private:
    float m_messageDisplayTime = 0.0f;
    std::string m_stolenName;
};

} // namespace bedrocktc
