# TODO / Version-specific work

1. Add a verified Minecraft build identifier and a second version dataset when another Bedrock version is supported.
2. Verify every migrated signature/offset against the target Bedrock binary.
3. Implement a version-correct camera projection for `worldToScreen`.
4. Implement version-correct world rendering for `worldLine`/`worldBox`.
5. Provide BTP with its final Preloader lifecycle/entrypoint adapter.
6. Run a real NDK r28c `arm64-v8a` build with the exact Preloader SDK and dependency versions.

These are intentionally explicit. No fake implementation or guessed Minecraft ABI has been introduced.
