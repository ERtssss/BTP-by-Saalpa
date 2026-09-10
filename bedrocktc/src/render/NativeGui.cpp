#include <bedrocktc/render/Render.hpp>
#include <android/log.h>
#include <btp/debug/Log.hpp>
#include <bedrocktc/hooks/Hooks.hpp>
#include <bedrocktc/memory/Signatures.hpp>
#include <bedrocktc/sdk/offsets/Core.hpp>
#include <pl/memory/Vtable.hpp>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>

namespace bedrocktc::render::native {
namespace {
#define BTC_LOGI(...) BTP_LOGI("NativeGui: " __VA_ARGS__)
#define BTC_LOGW(...) BTP_LOGW("NativeGui: " __VA_ARGS__)
#define BTC_LOGE(...) BTP_LOGE("NativeGui: " __VA_ARGS__)
#pragma pack(push, 4)
struct RectangleArea { float x0; float x1; float y0; float y1; };
struct TextMeasureData { float fontSize; float linePadding; bool renderShadow; bool showColorSymbol; bool hideHyphen; };
struct CaretMeasureData { int position; bool shouldRender; };
#pragma pack(pop)
struct Color { float r; float g; float b; float a; };
enum class TextAlignment : std::uint8_t { Left, Right, Center };

using DrawTextFn = void (*)(void*, void*, const RectangleArea&, const std::string&, const Color&, TextAlignment, float, const TextMeasureData&, const CaretMeasureData&);
using ScreenViewRenderFn = void (*)(void*, void*, void*, void*, void*, void*, void*, void*);
using GetClipFn = RectangleArea (*)(void*);
using FillRectFn = void (*)(void*, const RectangleArea&, const Color&, float);
using FlushTextFn = void (*)(void*, float, std::optional<float>);

std::atomic_bool gInitialized{false};
std::atomic_bool gOverlayDrawnThisFrame{false};
std::mutex gMutex;
OverlayCallback gCallback = nullptr;
void* gContext = nullptr;
void* gFont = nullptr;
DrawTextFn gDrawTextOriginal = nullptr;
ScreenViewRenderFn gScreenRenderOriginal = nullptr;

void drawTextHook(void* self, void* font, const RectangleArea& area, const std::string& text,
                  const Color& color, TextAlignment alignment, float alpha,
                  const TextMeasureData& measure, const CaretMeasureData& caret) {
    {
        std::lock_guard lock(gMutex);
        gContext = self;
        gFont = font;
    }
    static unsigned textLogs = 0;
    if (textLogs++ < 5) BTC_LOGI("DrawText hook reached: context=%p font=%p", self, font);
    if (gDrawTextOriginal) gDrawTextOriginal(self, font, area, text, color, alignment, alpha, measure, caret);

    // ScreenViewRender is entered before Minecraft starts drawing text, so the
    // render context is not available there yet. DrawText is the first reliable
    // point where both the context and font are known. Run the overlay once per
    // screen-render pass from here while the game's GL/UI rendering is active.
    if (!gOverlayDrawnThisFrame.exchange(true)) {
        OverlayCallback callback = nullptr;
        {
            std::lock_guard lock(gMutex);
            callback = gCallback;
        }
        if (callback) {
            BTC_LOGI("DrawText: overlay callback called");
            callback();
        } else {
            static unsigned missingCallbackLogs = 0;
            if (missingCallbackLogs++ < 5) BTC_LOGW("DrawText: overlay callback not installed");
        }
    }
}

void screenRenderHook(void* self, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8) {
    {
        std::lock_guard lock(gMutex);
        gContext = nullptr;
        gFont = nullptr;
    }
    gOverlayDrawnThisFrame.store(false);
    if (gScreenRenderOriginal) gScreenRenderOriginal(self, a2, a3, a4, a5, a6, a7, a8);

    OverlayCallback callback = nullptr;
    void* context = nullptr;
    void* font = nullptr;
    {
        std::lock_guard lock(gMutex);
        callback = gCallback;
        context = gContext;
        font = gFont;
    }
    static unsigned frameLogs = 0;
    if (frameLogs++ < 5) BTC_LOGI("ScreenViewRender hook: context=%p font=%p callback=%s", context, font, callback ? "yes" : "no");
    // The overlay is triggered from DrawText, after the RenderContext is valid.
    // Do not invoke it here because ScreenViewRender runs before DrawText.
    (void)callback;
    (void)context;
    (void)font;
}

void snapshotContext(void*& context, void*& font) {
    std::lock_guard lock(gMutex);
    context = gContext;
    font = gFont;
}

void** vtableOf(void* object) {
    return object ? *reinterpret_cast<void***>(object) : nullptr;
}

bool installVtableHook(const char* className, std::size_t slot, void* replacement, void** original) {
    const auto target = pl::memory::resolveVtableFunction(className, slot, "libminecraftpe.so");
    if (!target) return false;
    return hooks::install(reinterpret_cast<void*>(target), replacement, original) != nullptr;
}
}

bool initialize() {
    BTC_LOGI("NativeGui initialize begin");
    if (gInitialized.load()) { BTC_LOGI("NativeGui already initialized"); return true; }
    const auto screen = memory::resolve(memory::SignatureId::ScreenViewRender);
    if (!screen) { BTC_LOGE("ScreenViewRender signature not resolved"); return false; }

    bool screenOk = hooks::install(reinterpret_cast<void*>(screen), reinterpret_cast<void*>(screenRenderHook), reinterpret_cast<void**>(&gScreenRenderOriginal)) != nullptr;
    bool textOk = installVtableHook("24MinecraftUIRenderContext", sdk::offsets::VTable::MinecraftUIRenderContextDrawText,
                                    reinterpret_cast<void*>(drawTextHook), reinterpret_cast<void**>(&gDrawTextOriginal));
    BTC_LOGI("NativeGui hooks: ScreenViewRender=%s DrawText=%s", screenOk ? "OK" : "FAIL", textOk ? "OK" : "FAIL");
    if (!screenOk || !textOk) return false;
    gInitialized = true;
    BTC_LOGI("NativeGui initialize complete");
    return true;
}

void setOverlayCallback(OverlayCallback callback) {
    {
        std::lock_guard lock(gMutex);
        gCallback = callback;
    }
    BTC_LOGI("NativeGui overlay callback %s", callback ? "installed" : "cleared");
}

bool hasOverlayCallback() noexcept {
    std::lock_guard lock(gMutex);
    return gCallback != nullptr;
}

bool ready() noexcept { void* context = nullptr; void* font = nullptr; snapshotContext(context, font); return gInitialized.load() && context && font; }

Size size() noexcept {
    void* context = nullptr; void* font = nullptr;
    snapshotContext(context, font);
    if (!context) return {};
    void** vt = vtableOf(context);
    if (!vt || !vt[sdk::offsets::VTable::MinecraftUIRenderContextGetFullClippingRectangle]) return {};
    auto clip = reinterpret_cast<GetClipFn>(vt[sdk::offsets::VTable::MinecraftUIRenderContextGetFullClippingRectangle])(context);
    return {std::max(0.0f, clip.x1 - clip.x0), std::max(0.0f, clip.y1 - clip.y0)};
}

void fillRect(float x, float y, float w, float h, std::uint32_t rgba) {
    void* context = nullptr; void* font = nullptr;
    snapshotContext(context, font);
    if (!context || w <= 0.0f || h <= 0.0f) return;
    void** vt = vtableOf(context);
    if (!vt || !vt[sdk::offsets::VTable::MinecraftUIRenderContextFillRectangle]) return;
    const auto color = Color{
        ((rgba >> 16) & 0xFF) / 255.0f,
        ((rgba >> 8) & 0xFF) / 255.0f,
        (rgba & 0xFF) / 255.0f,
        ((rgba >> 24) & 0xFF) / 255.0f};
    const Size screen = size();
    const float sx = screen.width > 0.0f ? screen.width / 1156.0f : 1.0f;
    const float sy = screen.height > 0.0f ? screen.height / 704.0f : 1.0f;
    const RectangleArea area{x * sx, (x + w) * sx, y * sy, (y + h) * sy};
    reinterpret_cast<FillRectFn>(vt[sdk::offsets::VTable::MinecraftUIRenderContextFillRectangle])(gContext, area, color, 1.0f);
}

void text(float x, float y, float w, float h, float fontSize, std::string_view value, std::uint32_t rgba, bool shadow) {
    void* context = nullptr; void* font = nullptr;
    snapshotContext(context, font);
    if (!context || !font || value.empty()) return;
    void** vt = vtableOf(context);
    if (!vt || !vt[sdk::offsets::VTable::MinecraftUIRenderContextDrawText]) return;
    const auto color = Color{
        ((rgba >> 16) & 0xFF) / 255.0f,
        ((rgba >> 8) & 0xFF) / 255.0f,
        (rgba & 0xFF) / 255.0f,
        ((rgba >> 24) & 0xFF) / 255.0f};
    const Size screen = size();
    const float sx = screen.width > 0.0f ? screen.width / 1156.0f : 1.0f;
    const float sy = screen.height > 0.0f ? screen.height / 704.0f : 1.0f;
    const RectangleArea area{x * sx, (x + w) * sx, y * sy, (y + h) * sy};
    const TextMeasureData measure{fontSize * sy, 0.0f, shadow, false, false};
    const CaretMeasureData caret{0, false};
    std::string copy(value);
    reinterpret_cast<DrawTextFn>(vt[sdk::offsets::VTable::MinecraftUIRenderContextDrawText])(
        context, font, area, copy, color, TextAlignment::Left, 1.0f, measure, caret);
}

void flushText() {
    void* context = nullptr; void* font = nullptr;
    snapshotContext(context, font);
    if (!context) return;
    void** vt = vtableOf(context);
    if (!vt || !vt[sdk::offsets::VTable::MinecraftUIRenderContextFlushText]) return;
    reinterpret_cast<FlushTextFn>(vt[sdk::offsets::VTable::MinecraftUIRenderContextFlushText])(gContext, 0.0f, std::nullopt);
}

}
