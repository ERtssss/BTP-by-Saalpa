# BTP — Bedrock Tools Plus by Saalpa

BTP by Saalpa is an Android arm64-v8a LeviLauncher native mod. It combines the Bedrock Tools module set with a custom BTP GUI while keeping Minecraft-native infrastructure in Bedrock TC.

## Architecture

LeviLauncher → official Preloader Android SDK 0.2.2 → `libBTP.so` → BTP GUI + Bedrock TC.

Bedrock TC owns signatures, hooks, patches, memory, world/entity access, rendering adapters, events, input adapters, version abstraction and the 48 native modules. BTP owns presentation: custom GUI, module browser, HUD presentation, profiles, notifications and user settings.

## Build

Android NDK 28.2.13676358, CMake 3.22+, arm64-v8a. Preloader is fetched from `LiteLDev/preloader-android` tag `0.2.2` using CMake `FetchContent`.

```bash
cmake -S . -B build-arm64-v8a -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26
cmake --build build-arm64-v8a --target BTP levi_package --parallel
```

The resulting native library is `libBTP.so` and the packaging target produces a `.levipack` archive.

## SDK rule

No fake `pl/*` headers are included. The project uses only the public Preloader SDK surface available from the pinned 0.2.2 release.
