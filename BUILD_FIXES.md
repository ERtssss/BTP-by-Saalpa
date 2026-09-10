# BTP build compatibility fixes

Fixes the GitHub Actions include errors:

- `modules/ModuleRegistry.hpp` -> BedrockTC ModuleRegistry compatibility bridge.
- `core/memory/Hooks.hpp` -> BedrockTC hooks compatibility bridge.
- Explicit `bedrocktc/include/bedrocktc` include root in CMake.

The BedrockTC hook and module implementations themselves are unchanged.
