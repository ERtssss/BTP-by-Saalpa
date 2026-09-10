#include <btp/modules/ClientModules.hpp>
namespace btp::modules {
ClientModules& ClientModules::get() { static ClientModules instance; return instance; }
void ClientModules::initialize() {}
void ClientModules::shutdown() {}
Module& ClientModules::watermark() { return mModules[0]; }
Module& ClientModules::arrayList() { return mModules[1]; }
Module& ClientModules::customCrosshair() { return mModules[2]; }
Module& ClientModules::notifications() { return mModules[3]; }
Module& ClientModules::performanceMonitor() { return mModules[4]; }
const std::array<Module, 5>& ClientModules::all() const noexcept { return mModules; }
Module* ClientModules::find(std::string_view id) noexcept { for (auto& module : mModules) if (module.id == id) return &module; return nullptr; }
bool ClientModules::setEnabled(std::string_view id, bool enabled) noexcept { auto* module = find(id); if (!module) return false; module->enabled = enabled; return true; }
}
