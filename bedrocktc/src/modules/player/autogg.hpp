#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class AutoGG : public ::bedrocktc::Module {
public:
    static AutoGG* instance;
    
    AutoGG();
    
    void onInit() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

    
    std::string ggMessage = "gg";
};

} // namespace bedrocktc
