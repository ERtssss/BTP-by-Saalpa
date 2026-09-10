#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <string>

namespace bedrocktc {



class AutoReQ : public ::bedrocktc::Module {
public:
    AutoReQ();
    ~AutoReQ() override;
    
    void onInit() override;
    
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;
    
    
    bool soloMode = true;
    bool teamElimination = true;
    bool gameOver = false;
    bool roleMurderer = false;
    bool roleSheriff = false;
    bool roleInnocent = false;
    bool roleHider = false;
    bool roleSeeker = false;
    bool roleDeath = false;
    bool roleRunner = false;

    int cooldownMs = 3000;

    static AutoReQ* instance;
};

} // namespace bedrocktc
