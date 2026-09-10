#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class TntTimerModule : public ::bedrocktc::Module {
public:
    TntTimerModule();
    ~TntTimerModule() override;

    void onInit() override;
};

} // namespace bedrocktc
