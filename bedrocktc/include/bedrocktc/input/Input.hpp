#pragma once
#include <pl/Input.hpp>
namespace bedrocktc::input {
using MouseEvent = pl::input::MouseEvent;
using MouseCallback = bool(*)(const MouseEvent&);
inline void registerMouseCallback(MouseCallback callback) {
    pl::input::registerMouseCallback(callback);
}
inline void showKeyboard() {
    pl::input::showKeyboard();
}
inline void hideKeyboard() {
    pl::input::hideKeyboard();
}
}
