# Public API

## Bootstrap

```cpp
#include <bedrocktc/BedrockTC.hpp>

bedrocktc::core::setConfigPath(path);
bedrocktc::core::initialize();
```

`initialize()` returns `false` if Minecraft is not loaded, signatures cannot be resolved, or the existing game hook installation fails.

## Client

```cpp
auto* player = bedrocktc::client::getLocalPlayer();
auto* client = bedrocktc::client::get();
```

`Player` is the real migrated `sdk::Player` wrapper from the source project.

## World/entity

```cpp
auto* level = bedrocktc::world::getLevel();
auto entities = bedrocktc::world::getEntities();
```

The entity list uses the source snapshot's `ActorManagerList` signature. If that signature is unavailable, the result is empty.

## Modules

```cpp
auto& registry = bedrocktc::ModuleRegistry::get();
registry.initialize();

if (auto* module = registry.find("bedrocktc.Fullbright")) {
    module->toggle();
}
```

`Module` provides ID, name, description, category, enabled state, `enable`, `disable`, `toggle`, lifecycle and configuration hooks.

## Events

The existing typed `EventBus` is retained and exposed through `bedrocktc::events`.

## Memory/hooks/patches

- `bedrocktc::memory` — safe wrappers around the migrated low-level memory helpers
- `bedrocktc::hooks` — Preloader hook adapter
- `bedrocktc::patches::apply()` — migrated memory patch operation
- `bedrocktc::signatures` — signature facade

## Render

`bedrocktc::render::text`, `rect`, `line`, `image` and `submit` create/use the real Preloader `DrawCommand` representation.

World projection helpers explicitly report unsupported state until a version-correct camera implementation is added.
