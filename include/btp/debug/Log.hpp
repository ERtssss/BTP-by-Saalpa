#pragma once

#include <android/log.h>
#include <filesystem>
#include <string>

namespace btp::debug {

void setLogFile(const std::filesystem::path& path);
void clearLogFile();
void write(int priority, const char* format, ...);

}

#ifndef BTP_DEBUG_LOG
#define BTP_DEBUG_LOG 1
#endif

#if BTP_DEBUG_LOG
#define BTP_LOGI(...) ::btp::debug::write(ANDROID_LOG_INFO, __VA_ARGS__)
#define BTP_LOGW(...) ::btp::debug::write(ANDROID_LOG_WARN, __VA_ARGS__)
#define BTP_LOGE(...) ::btp::debug::write(ANDROID_LOG_ERROR, __VA_ARGS__)
#else
#define BTP_LOGI(...) ((void)0)
#define BTP_LOGW(...) ((void)0)
#define BTP_LOGE(...) ((void)0)
#endif
