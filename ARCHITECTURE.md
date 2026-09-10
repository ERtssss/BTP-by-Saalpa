# BTP by Saalpa — clean architecture

```text
LeviLauncher
    ↓
Preloader Android SDK 0.2.2
    ↓
libBTP.so
    ├── BTP Client
    │   ├── GUI
    │   ├── HUD presentation
    │   ├── Profiles
    │   ├── Config
    │   └── Notifications
    │
    └── Bedrock TC
        ├── SDK
        ├── Memory / Hooks
        ├── Signatures / Offsets
        ├── Events / Input
        ├── Rendering bridge
        ├── World / Entity / Network
        └── Built-in modules
```

## Responsibility

**BTP Client** contains only user-facing functionality.

**Bedrock TC** is the technical core. It owns the Minecraft integration and the complete built-in module registry.

There is no second BTP module registry, no legacy launcher-side module menu and no duplicate game-hook layer.
