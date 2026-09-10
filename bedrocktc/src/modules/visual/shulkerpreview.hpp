#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {


// Shulker preview module — thanks to Kashifro
// GitHub: https://github.com/Kashifro


class ShulkerPreviewModule : public ::bedrocktc::Module {
public:
    float m_tintIntensity = 2.0f;
    float m_positionX = 8.0f;
    float m_positionY = 8.0f;
    bool m_followSelectedShulker = false;

    ShulkerPreviewModule();
    ~ShulkerPreviewModule() override;

    void onInit() override;
    void onDisable() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

private:
    bool m_hooksInstalled = false;
};

void ShulkerPreviewHandleContainerDestroyed(void* controller);

} // namespace bedrocktc
