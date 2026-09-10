# Bedrock TC

Bedrock TC is the low-level native core used by BTP. It consumes the public Preloader SDK and owns signatures, hooks, patches, native access, events, input and rendering adapters.

The canonical build is the root BTP CMake project, which fetches Preloader 0.2.2 and links the `preloader` target into `BTP`.

No `pl/*` headers are copied into this repository.
