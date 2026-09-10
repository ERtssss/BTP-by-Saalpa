#pragma once
#include <bedrocktc/events/EventBus.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

namespace btp::hud {
struct Element { std::string id; bool visible = true; float x = 10.f, y = 10.f, scale = 1.f; };
class Hud {
public:
    static Hud& get();
    void initialize();
    void shutdown();
    void draw();
    Element& element(std::string_view id);
    const std::unordered_map<std::string, Element>& elements() const noexcept;
    void reset(std::string_view id);
private:
    std::unordered_map<std::string, Element> mElements;
    bedrocktc::events::Subscription mFrameSubscription = 0;
};
}
