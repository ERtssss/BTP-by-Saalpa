#pragma once
#include <bedrocktc/modules/Module.hpp>

namespace bedrocktc {



class ChatTimestampsModule : public ::bedrocktc::Module {
public:
    ChatTimestampsModule();
    ~ChatTimestampsModule() override;

    void onInit() override;
};

} // namespace bedrocktc
