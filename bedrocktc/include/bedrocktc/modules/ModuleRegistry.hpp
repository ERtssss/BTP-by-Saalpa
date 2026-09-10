#pragma once
#include "Module.hpp"
#include <pl/ModMenu.hpp>
#include <memory>
#include <type_traits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bedrocktc {



#define PLModMenu_DrawCommand pl::modmenu::DrawCommand
#define PLModMenu_DrawCommandType pl::modmenu::DrawCommandType
#define PL_DRAW_TEXT pl::modmenu::DrawCommandType::Text
#define PL_DRAW_RECT pl::modmenu::DrawCommandType::Rect
#define PL_DRAW_LINE pl::modmenu::DrawCommandType::Line
#define PL_DRAW_RECT_FILLED pl::modmenu::DrawCommandType::RectFilled
#define PL_DRAW_CIRCLE_FILLED pl::modmenu::DrawCommandType::CircleFilled
#define PL_DRAW_TRIANGLE_FILLED pl::modmenu::DrawCommandType::TriangleFilled
#define PL_DRAW_IMAGE pl::modmenu::DrawCommandType::Image

inline void submitDrawCommands(std::string_view moduleId, const std::vector<PLModMenu_DrawCommand>& commands) {
    pl::modmenu::submitDrawCommands(moduleId, commands);
}

class ModuleRegistry {
public:
    static ModuleRegistry& get();

    template <class T, class... Args>
    T& emplace(Args&&... args) {
        static_assert(std::is_base_of_v<Module, T>, "Every BedrockTC module must inherit bedrocktc::Module");
        auto module = std::make_unique<T>(std::forward<Args>(args)...);
        auto* raw = module.get();
        if (auto it=mById.find(raw->moduleId); it != mById.end()) return *static_cast<T*>(it->second);
        mById.emplace(raw->moduleId, raw);
        mView.push_back(raw);
        mOwned.push_back(std::move(module));
        return *raw;
    }

    bool registerModule(std::unique_ptr<Module> module);
    bool unregisterModule(std::string_view id);
    Module* find(std::string_view id) const;
    const std::vector<Module*>& modules() const;
    void initialize();
    void shutdown();
    void registerModMenu(std::string_view ownerModId);
    void unregisterModMenu();
    void onFrame();
    bool onMouseEvent(int button, bool isDown);
    void setKeybindBlocked(bool blocked);
    bool keybindBlocked() const;

private:
    struct StringHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view value) const noexcept {
            return std::hash<std::string_view>{}(value);
        }
    };

    std::vector<std::unique_ptr<Module>> mOwned;
    std::vector<Module*> mView;
    std::unordered_map<std::string, Module*, StringHash, std::equal_to<>> mById;
    bool mInitialized = false;
    bool mKeybindBlocked = false;
};

void registerAllModules();

} // namespace bedrocktc
