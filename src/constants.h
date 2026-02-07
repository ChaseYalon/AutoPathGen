#pragma once

// ALL DISTANCES IN METERS
// ALL TIMES IN SECONDS
// UNLESS OTHERWISE SPECIFIED

#include <cstdio>

#ifdef NDEBUG
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(...)                                                         \
  do {                                                                         \
    printf("[DEBUG] ");                                                        \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  } while (0)
#endif

#define FIELD_WIDTH 16.540988f
#define FIELD_HEIGHT 8.069326f
#define SIDE_PANEL_WIDTH 500
