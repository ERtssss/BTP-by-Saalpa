# BTP Architecture

```text
LeviLauncher
    ↓
Preloader Android SDK 0.2.2
    ↓
libBTP.so
    ├── BTP GUI / presentation
    └── Bedrock TC / native technical core
             ↓
       Minecraft Bedrock
```

## BTP

Owns the custom Bedrock Tools-style GUI, module browser, search, profiles, notifications, HUD presentation and user-facing settings.

## Bedrock TC

Owns Minecraft-native infrastructure and the complete 48-module registry. BTP never duplicates those module objects.

## Cleanup

Native hooks and patches are owned by the TC lifecycle. Mod Menu registrations and draw-command ownership are cleaned up by their respective owners. Input callbacks follow the limitations of the pinned public SDK and use static runtime state so they cannot retain destroyed GUI objects.
