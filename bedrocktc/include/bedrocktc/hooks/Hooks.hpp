#pragma once
#include <cstdint>
#include <algorithm>
#include <dlfcn.h>
#include <memory>
#include <mutex>
#include <vector>
#include <pl/memory/Hook.hpp>
namespace bedrocktc::hooks {
using LibraryHandle = void*;
struct State { pl::memory::HookHandle handle; };
using Handle = State*;
namespace detail {
inline std::mutex gMutex;
inline std::vector<Handle> gHandles;
inline void track(Handle h) { std::lock_guard lock(gMutex); gHandles.push_back(h); }
inline void untrack(Handle h) { std::lock_guard lock(gMutex); gHandles.erase(std::remove(gHandles.begin(), gHandles.end(), h), gHandles.end()); }
}
inline LibraryHandle openLibrary(const char* libraryName) { if (!libraryName) return nullptr; void* handle = dlopen(libraryName, RTLD_NOW | RTLD_NOLOAD); return handle ? handle : dlopen(libraryName, RTLD_NOW); }
inline int closeLibrary(LibraryHandle handle) { return handle ? dlclose(handle) : 0; }
inline std::uintptr_t symbol(LibraryHandle handle, const char* name) { return (!handle || !name) ? 0 : reinterpret_cast<std::uintptr_t>(dlsym(handle, name)); }
inline Handle install(void* target, void* detour, void** original, pl::memory::HookPriority priority = pl::memory::HookPriority::Normal) {
    if (!target || !detour) return nullptr;
    auto state = std::make_unique<State>();
    state->handle = pl::memory::HookHandle(target, detour, original, priority);
    if (!state->handle.installed()) return nullptr;
    Handle raw = state.release(); detail::track(raw); return raw;
}
inline void remove(Handle hook) { if (!hook) return; detail::untrack(hook); hook->handle.reset(); delete hook; }
inline void removeAll() {
    std::vector<Handle> handles;
    { std::lock_guard lock(detail::gMutex); handles.swap(detail::gHandles); }
    for (auto* hook : handles) { if (!hook) continue; hook->handle.reset(); delete hook; }
}
}
