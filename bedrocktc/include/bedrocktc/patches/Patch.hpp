#pragma once
#include <pl/memory/Patch.hpp>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
namespace bedrocktc::patches {
using Handle = std::unique_ptr<pl::memory::PatchHandle>;
inline Handle apply(std::uintptr_t address, std::span<const std::uint8_t> bytes, std::string name) {
    if (!address || bytes.empty()) return {};
    auto handle = std::make_unique<pl::memory::PatchHandle>(address, bytes, std::move(name));
    if (!handle->applied()) return {};
    return handle;
}
template<class T> inline Handle apply(std::uintptr_t address, const T& value, std::string name) {
    return apply(address, std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(&value), sizeof(T)), std::move(name));
}
inline void remove(Handle& handle) { if (handle) handle->reset(); handle.reset(); }
}
