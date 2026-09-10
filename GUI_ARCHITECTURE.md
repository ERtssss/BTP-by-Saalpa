# BTP GUI architecture

BTP now loads its GUI the same way as the working BedrockTools project.

## Lifecycle

1. `Runtime::load()` stores the NativeMod context and config paths.
2. `Runtime::enable()` calls `BedrockTC::initializeIfMinecraftLoaded()`.
3. If `libminecraftpe.so` is not loaded yet, BedrockTC installs a `dlopen` watcher.
4. When Minecraft loads, BedrockTC resolves signatures and installs game hooks.
5. BTP registers and initializes all modules.
6. BTP loads its configuration.
7. `ModuleRegistry::registerModMenu()` creates the GUI with `pl::modmenu::ModuleBuilder`.

This is the same launcher-side GUI model used by BedrockTools. The old Dear ImGui/NativeGui rendering path is not part of the active BTP runtime.


## BTP Menu (current)
BTP uses its own in-game native Minecraft UI. LeviLauncher ModMenu registration is intentionally disabled. The NativeGui bridge is gated by ClientInstance::localPlayer(), so the BTP overlay is not drawn on title/login/launcher screens.
