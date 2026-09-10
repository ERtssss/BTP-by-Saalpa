# Architecture

## Responsibility split

**Bedrock TC owns**
- Minecraft/native access
- version data, signatures and offsets
- hooks, memory and patches
- client/world/entity wrappers
- event transport
- input/network adapters
- render command adapter
- configuration infrastructure
- Module and ModuleRegistry
- all 47 modules inherited from the supplied BedrockTools tree

**BTP owns**
- product/client UX
- GUI composition and screens
- user-facing orchestration
- BTP specific features
- final shared-library entry point and Preloader lifecycle integration

BTP should not include BedrockTools implementation headers or reach into `src/`.

## Version layer

Version-dependent signatures and offsets live under:

`include/bedrocktc/version/`

The old `sdk/offsets/*` paths are compatibility wrappers around the version layer. Signature resolution is similarly exposed through `memory/Signatures.hpp` as a compatibility facade.

There is currently one migrated source snapshot. It is not assigned an invented Minecraft version number. Updating to another Bedrock build should add a new `VersionData`/signature-offset dataset rather than spreading offsets through feature code.

## Runtime

`bedrocktc::core::initialize()` expects `libminecraftpe.so` to already be loaded. It resolves signatures, installs the existing game hooks, registers the 47 modules, wires the event bridge and loads configuration.

This avoids embedding a Preloader `Mod` object or launcher GUI inside the static core.

## Rendering

Existing modules continue to use the real Preloader ModMenu `DrawCommand` type. `bedrocktc::render::submit()` is a thin adapter.

`worldToScreen`, `worldLine` and `worldBox` are explicitly non-functional in the supplied source snapshot because no reliable projection implementation was present. They return `false` rather than pretending to work.

## Module registration

`registerAllModules()` is idempotent. `ModuleRegistry::registerModule()` rejects duplicate IDs. `initialize()` and `shutdown()` are lifecycle boundaries.

## Public/private boundary

Everything under `include/bedrocktc/` is eligible for consumers. `src/` is implementation detail. Legacy launcher files from the supplied project are not part of the target.
