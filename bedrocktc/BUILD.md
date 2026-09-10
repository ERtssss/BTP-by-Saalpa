# Bedrock TC build

Build through the root BTP CMake project. The root project fetches LiteLDev/preloader-android `0.2.2`, Boost.PFR, magic_enum, fmt, nlohmann/json, GLM and EnTT.

```sh
cmake -S . -B build-arm64-v8a -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_HOME/ndk/28.2.13676358/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-28
cmake --build build-arm64-v8a --target BTP levi_package
```
