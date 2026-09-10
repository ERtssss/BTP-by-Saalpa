#pragma once
#include <pl/ModMenu.hpp>
#include <string_view>
#include <cstdint>
#include <bedrocktc/sdk/Types.hpp>
#include <vector>
namespace bedrocktc::render {
using OverlayCallback = void(*)();
struct Size { float width{}; float height{}; };
namespace native {
bool initialize();
void setOverlayCallback(OverlayCallback callback);
bool hasOverlayCallback() noexcept;
bool ready() noexcept;
Size size() noexcept;
void fillRect(float x, float y, float w, float h, std::uint32_t rgba);
void text(float x, float y, float w, float h, float fontSize, std::string_view value, std::uint32_t rgba = 0xFFFFFFFF, bool shadow = false);
void flushText();
}

using DrawCommand=pl::modmenu::DrawCommand;
using DrawCommandType=pl::modmenu::DrawCommandType;
inline void submit(std::string_view owner,const std::vector<DrawCommand>& commands){pl::modmenu::submitDrawCommands(owner,commands);}
inline void text(std::string_view owner,float x,float y,float w,float h,float size,std::string_view value,std::uint32_t color=0xFFFFFFFF){
 DrawCommand c{}; c.type=DrawCommandType::Text;c.x=x;c.y=y;c.w=w;c.h=h;c.size=size; c.color=color; c.text=value.data(); submit(owner,{c});
}
inline void rect(std::string_view owner,float x,float y,float w,float h,std::uint32_t color,bool filled=true){
 DrawCommand c{}; c.type=filled?DrawCommandType::RectFilled:DrawCommandType::Rect;c.x=x;c.y=y;c.w=w;c.h=h;c.color=color; submit(owner,{c});
}
inline void line(std::string_view owner,float x,float y,float w,float h,std::uint32_t color){
 DrawCommand c{}; c.type=DrawCommandType::Line;c.x=x;c.y=y;c.w=w;c.h=h;c.color=color; submit(owner,{c});
}
inline bool worldToScreen(const sdk::Vec3&, sdk::Vec2&) { return false; } // Version-specific projection is not present in the source SDK.
inline bool worldLine(std::string_view,float,float,float,float,float,float,std::uint32_t) { return false; } // TODO: version-specific world renderer
inline bool worldBox(std::string_view,float,float,float,float,float,float,std::uint32_t) { return false; } // TODO: version-specific world renderer
inline void image(std::string_view owner,float x,float y,float w,float h,std::string_view imageId){
 DrawCommand c{}; c.type=DrawCommandType::Image;c.x=x;c.y=y;c.w=w;c.h=h;c.imageId=imageId.data(); submit(owner,{c});
}
}
