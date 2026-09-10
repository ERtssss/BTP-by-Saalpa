#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
namespace bedrocktc {
enum class ModuleCategory { Client, Combat, Movement, Render, HUD, Misc };
class Module {
public:
 const char* name; const char* description; std::string moduleId;
 ModuleCategory category = ModuleCategory::Misc;
 bool masterEnabled=false,keybindActive=true,enabled=false,showInMenu=true,hideInHudEditor=false; int keybind=0;
 Module(const char* n,const char* d,ModuleCategory c=ModuleCategory::Misc):name(n),description(d),moduleId(std::string("bedrocktc.")+n),category(c){}
 virtual ~Module()=default;
 virtual void onInit(){} virtual void onEnable(){} virtual void onDisable(){} virtual void onFrame(){}
 virtual bool onMouseEvent(int,bool){return false;}
 virtual bool onMenuConfigChanged(std::string_view,std::string_view){return false;}
 virtual bool showInLegacyMenu(std::string_view) const{return true;}
 virtual void onMenuRegistered(){}
 virtual void onKeybindEvent(const std::string& key,bool down){if(key=="keybind"&&down)setKeybindActive(!keybindActive);}
 void enable(){setMasterEnabled(true);} void disable(){setMasterEnabled(false);} void toggle(){setMasterEnabled(!masterEnabled);}
 bool isEnabled() const noexcept{return enabled;}
 void setMasterEnabled(bool v){if(masterEnabled==v)return;masterEnabled=v;updateEnabledState();}
 void setKeybindActive(bool v){if(keybindActive==v)return;keybindActive=v;updateEnabledState();}
 void updateEnabledState(){bool n=masterEnabled&&keybindActive;if(n!=enabled){enabled=n;if(enabled)onEnable();else onDisable();}}
 virtual void loadConfig(const nlohmann::json& j){if(j.contains("keybind"))keybind=j["keybind"].get<int>();if(j.contains("keybindActive"))keybindActive=j["keybindActive"].get<bool>();if(j.contains("masterEnabled"))masterEnabled=j["masterEnabled"].get<bool>();updateEnabledState();}
 virtual void saveConfig(nlohmann::json& j){j["keybind"]=keybind;j["keybindActive"]=keybindActive;j["masterEnabled"]=masterEnabled;}
};
}
