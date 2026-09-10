#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class NoTouchBorderModule : public ::bedrocktc::Module {
public:
    NoTouchBorderModule();
    void onInit() override;
};

} // namespace bedrocktc
