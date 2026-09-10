#pragma once
#if defined(__GNUC__) || defined(__clang__)
#define BEDROCKTC_API __attribute__((visibility("default")))
#else
#define BEDROCKTC_API
#endif
