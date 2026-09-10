#pragma once

#include <bedrocktc/Api.hpp>
#include <bedrocktc/memory/Signatures.hpp>

namespace bedrocktc::sdk {

template <class Function>
Function function(memory::SignatureId id, const api::ApiV1* runtime = nullptr) {
    if (!runtime) runtime = api::getApi();
    const auto address = api::resolve(id, runtime);
    return address ? reinterpret_cast<Function>(address) : nullptr;
}

}
