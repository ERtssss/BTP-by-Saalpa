#pragma once
#include <bedrocktc/modules/Module.hpp>
#include <string>

namespace bedrocktc {



class QuickLootModule : public ::bedrocktc::Module {
public:
    QuickLootModule();
    void onInit() override;

private:
    using HandleAutoPlaceFn = void (*)(void*, int, const std::string&, int);

    HandleAutoPlaceFn m_handleAutoPlace = nullptr;
    bool m_transferring = false;
};

} // namespace bedrocktc
