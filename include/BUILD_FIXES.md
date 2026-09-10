# BTP build compatibility fixes

This revision fixes the include-path failures reported by GitHub Actions:

- `modules/ModuleRegistry.hpp` is provided as a compatibility bridge to `bedrocktc/modules/ModuleRegistry.hpp` and exposes `bedrocktoolsplus::ModuleRegistry`.
- `core/memory/Hooks.hpp` is provided as a compatibility bridge to `bedrocktc/hooks/Hooks.hpp` and aliases `bedrocktoolsplus::hooks`.
- The root CMake target explicitly includes `bedrocktc/include/bedrocktc`.

These changes do not alter the BedrockTC hook implementation or module behavior.
