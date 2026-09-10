# Public Preloader SDK Audit

Pinned SDK: `LiteLDev/preloader-android` tag `0.2.2`.

Audited areas: Native Mod entry/lifecycle, Mod Menu, Config, Input, Hook, Signature, Patch, public memory types/macros and public `include/pl` declarations.

BTP does not ship replacement `pl/*` headers and does not implement a second hook/signature/patch framework.

Where the public 0.2.2 SDK has no corresponding operation, BTP does not invent one. In particular, Input callback registration is available publicly but there is no public unregister operation in the pinned header; callbacks therefore target process-lifetime static runtime state.

ABI note: native mods must be rebuilt against the same Preloader SDK/runtime ABI. LeviLaunchroid has documented ABI sensitivity around public struct changes, so the pinned SDK is intentional.
