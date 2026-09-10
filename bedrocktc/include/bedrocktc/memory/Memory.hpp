#pragma once
#include <bedrocktc/sdk/Memory.hpp>
#include <utility>
namespace bedrocktc::memory {
using sdk::makeWritableExecutable; using sdk::patchMemory;
template<class T> T& field(void* o,std::size_t off){return sdk::field<T>(o,off);}
template<class T> const T& field(const void* o,std::size_t off){return sdk::field<T>(o,off);}
template<class R,class... A> R virtualCall(void* i,std::size_t n,A&&... a){return sdk::virtualCall<R>(i,n,std::forward<A>(a)...);}
}
