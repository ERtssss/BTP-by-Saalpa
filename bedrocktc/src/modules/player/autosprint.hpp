#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class AutoSprintModule : public ::bedrocktc::Module {
public:
    AutoSprintModule();
    void onInit() override;
};

} // namespace bedrocktc
