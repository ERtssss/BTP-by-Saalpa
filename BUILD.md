# BTP Native Build

BTP is a real LeviLauncher Android native mod and uses LiteLDev/preloader-android 0.2.2 through CMake FetchContent. The official template also uses release 0.2.2 and Android NDK 28.2.13676358.

```sh
cmake -S . -B build-arm64-v8a -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_HOME/ndk/28.2.13676358/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-28
cmake --build build-arm64-v8a --target BTP levi_package
```

The generated native library is `libBTP.so`; the package target produces a `.levipack`.

No Preloader headers are copied into this repository and no fake `pl/*` implementation is used.
