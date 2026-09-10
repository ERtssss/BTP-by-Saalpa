# BTP-by-Saalpa 1.0.0 — GUI fix

This package is based on the uploaded `BTP-by-Saalpa-1.0.0-btp-menu.zip`.

## GUI fixes
- The BTP overlay is now drawn after Minecraft's `ScreenView::render`, using the same render-context capture pattern already used by the ShulkerPreview module.
- The overlay is no longer recursively invoked from inside the `DrawText` hook.
- The GUI no longer depends on `ClientInstance::localPlayer()` being non-null before it can render.
- BACK / ESC / F8 key handling no longer silently fails because `localPlayer()` is unavailable.
- Native GUI initialization keeps both hooks but treats the DrawText hook as the context provider and ScreenViewRender as the safe post-render draw point.

## LeviPack fixes
- The CMake shared-library output now exactly matches the manifest: `libBTP.so`.
- Added a `levi_package` CMake target that creates a real `.levipack` with `manifest.json`, `libBTP.so`, and `icon.png`.
- Manifest branding is `Bedrock Tools Plus` by `Saalpa`.
