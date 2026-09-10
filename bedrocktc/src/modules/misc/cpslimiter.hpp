#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <atomic>
#include <chrono>
#include <deque>
#include <mutex>

namespace bedrocktc {



class CpsLimiterModule : public ::bedrocktc::Module {
public:
    CpsLimiterModule();

    void onEnable() override;
    void onDisable() override;
    bool onMouseEvent(int button, bool isDown) override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

private:
    struct ButtonState {
        bool physicalDown = false;
        bool forwardedDown = false;
        std::chrono::steady_clock::time_point nextAllowed{};
        std::deque<std::chrono::steady_clock::time_point> allowedClicks;
    };

    bool handleButton(ButtonState& state, bool isDown, bool limited, int cps);
    void resetRateState(ButtonState& state);

    std::atomic_bool m_active{false};
    std::atomic_bool m_limitLeft{true};
    std::atomic_int m_limitLeftCps{10};
    std::atomic_bool m_limitRight{true};
    std::atomic_int m_limitRightCps{10};
    ButtonState m_leftState;
    ButtonState m_rightState;
    std::mutex m_stateMutex;
};

} // namespace bedrocktc
