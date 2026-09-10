cmake_minimum_required(VERSION 3.22)
project(BedrockTC LANGUAGES CXX)

if(NOT ANDROID)
    message(FATAL_ERROR "BedrockTC is an Android/arm64-v8a core. Configure with the Android NDK toolchain.")
endif()
if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "BedrockTC requires arm64-v8a.")
endif()
if(NOT DEFINED LEVI_PRELOADER_INCLUDE_DIR)
    message(FATAL_ERROR "LEVI_PRELOADER_INCLUDE_DIR must be provided by the root FetchContent Preloader dependency.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file(GLOB_RECURSE BEDROCKTC_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(BedrockTCCore STATIC ${BEDROCKTC_SOURCES})
target_include_directories(BedrockTCCore
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PUBLIC "${LEVI_PRELOADER_INCLUDE_DIR}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
    PRIVATE "${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include"
    PRIVATE "${CMAKE_BINARY_DIR}/_deps/glm-src"
    PRIVATE "${CMAKE_BINARY_DIR}/_deps/entt-src/src"
    PRIVATE "${CMAKE_BINARY_DIR}/_deps/boost_pfr-src/include"
    PRIVATE "${CMAKE_BINARY_DIR}/_deps/magic_enum-src/include"
    PRIVATE "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")

target_link_libraries(BedrockTCCore PUBLIC
    nlohmann_json::nlohmann_json
    glm::glm
    EnTT::EnTT
    preloader)

target_compile_options(BedrockTCCore PRIVATE
    -fPIC -ffunction-sections -fdata-sections -flto -fexceptions
    -fvisibility=hidden -fvisibility-inlines-hidden -fno-rtti -Oz)
target_link_options(BedrockTCCore PRIVATE
    -flto -Wl,--gc-sections -Wl,--icf=all
    -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384)
target_link_libraries(BedrockTCCore PUBLIC android log EGL GLESv3 GLESv2)
