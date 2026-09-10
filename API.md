# BTP API

Public umbrella include:

```cpp
#include <btp/BTP.hpp>
```

## Module manager

```cpp
using namespace btp::core;
auto* module = ModuleManager::get().find("bedrocktc.Fullbright");
ModuleManager::get().setEnabled("bedrocktc.Fullbright", true);
ModuleManager::get().toggle("bedrocktc.Fullbright");
auto list = ModuleManager::get().search("full", Category::Visuals);
```

`ModuleManager` operates on objects owned by `bedrocktc::ModuleRegistry`.

## Runtime

```cpp
auto& runtime = btp::core::Runtime::get();
runtime.load(configDirectory);
runtime.enable();
```

The Preloader-specific `ModContext` remains confined to `src/main.cpp`.

## HUD

```cpp
auto& element = btp::hud::Hud::get().element("watermark");
element.visible = true;
element.x = 20.0f;
element.y = 20.0f;
element.scale = 1.0f;
```

## Profiles

```cpp
btp::profiles::Profiles::get().save("pvp");
btp::profiles::Profiles::get().load("pvp");
```

## Notifications

```cpp
btp::core::Notifications::get().push(
    btp::core::NotificationType::Success,
    "Fullbright enabled");
```
