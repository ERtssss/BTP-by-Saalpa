cmake_minimum_required(VERSION 3.22)
project(BTP VERSION 1.0.0 LANGUAGES CXX)

if(NOT ANDROID)
    message(FATAL_ERROR "BTP requires the Android NDK toolchain.")
endif()
if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "BTP supports arm64-v8a only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG 0.2.2
    SOURCE_SUBDIR cmake/preloader-sdk
)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_Declare(boost_pfr
    GIT_REPOSITORY https://github.com/boostorg/pfr.git
    GIT_TAG boost-1.88.0
)
FetchContent_Declare(magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG v0.9.7
)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_Declare(entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.15.0
)

set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(
    preloader_android
    nlohmann_json
    boost_pfr
    magic_enum
    fmt
    glm
    entt
)

FetchContent_GetProperties(preloader_android)
set(LEVI_PRELOADER_INCLUDE_DIR "${preloader_android_SOURCE_DIR}/include")

if(NOT TARGET preloader)
    add_subdirectory("${preloader_android_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/_preloader_android")
endif()

add_subdirectory(bedrocktc)

# BTP owns only the client/presentation layer.
# Bedrock TC owns Minecraft hooks, SDK, events, rendering bridge and modules.
set(BTP_SOURCES
    src/main.cpp
    src/core/DebugLog.cpp
    src/core/Notifications.cpp
    src/core/Runtime.cpp
    src/core/ModuleManager.cpp
    src/config/Config.cpp
    src/gui/Gui.cpp
    src/hud/Hud.cpp
    src/modules/ClientModules.cpp
    src/profiles/Profiles.cpp
)

add_library(BTPClient SHARED ${BTP_SOURCES})
set_target_properties(BTPClient PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "BTP"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

target_include_directories(BTPClient PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/bedrocktc/include"
    "${LEVI_PRELOADER_INCLUDE_DIR}"
)

target_link_libraries(BTPClient PRIVATE
    BedrockTCCore
    preloader
    nlohmann_json::nlohmann_json
    Boost::pfr
    magic_enum::magic_enum
    fmt::fmt
    android log EGL GLESv3 GLESv2
)

target_compile_options(BTPClient PRIVATE
    -fPIC
    -ffunction-sections
    -fdata-sections
    -flto
    -fexceptions
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fno-rtti
    -Oz
)

target_link_options(BTPClient PRIVATE
    -flto
    -Wl,--gc-sections
    -Wl,--icf=all
    -Wl,-z,max-page-size=16384
    -Wl,-z,common-page-size=16384
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    @ONLY
)
