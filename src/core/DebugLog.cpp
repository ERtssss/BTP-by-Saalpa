#include <btp/debug/Log.hpp>
#include <android/log.h>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>

namespace btp::debug {
namespace {
std::mutex gMutex;
std::filesystem::path gLogFile;

const char* levelName(int priority) {
    switch (priority) {
        case ANDROID_LOG_ERROR: return "E";
        case ANDROID_LOG_WARN: return "W";
        default: return "I";
    }
}
}

void setLogFile(const std::filesystem::path& path) {
    std::lock_guard lock(gMutex);
    gLogFile = path;
    std::error_code ec;
    if (!gLogFile.parent_path().empty()) std::filesystem::create_directories(gLogFile.parent_path(), ec);
    std::ofstream out(gLogFile, std::ios::app);
    if (out) out << "\n=== BTP debug session started ===\n";
}

void clearLogFile() {
    std::lock_guard lock(gMutex);
    gLogFile.clear();
}

void write(int priority, const char* format, ...) {
    char message[2048]{};
    va_list args;
    va_start(args, format);
    std::vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    __android_log_print(priority, "BTP", "%s", message);

    std::lock_guard lock(gMutex);
    if (gLogFile.empty()) return;
    std::ofstream out(gLogFile, std::ios::app);
    if (!out) return;

    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&time, &tm);
    char stamp[32]{};
    std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm);
    out << stamp << " [" << levelName(priority) << "] " << message << '\n';
}
}
