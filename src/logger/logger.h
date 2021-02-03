#pragma once

#include <cstdio>

#define UNIQUE_NAME __FILE__##__LINE__
#define LOGF(fmt, ...) printf("\n" fmt, __VA_ARGS__)
#define DEBUG_BREAK(fmt, ...)                                 \
{                                                             \
  LOGF("%s %d: " fmt, __FILE__, __LINE__, __VA_ARGS__);          \
  static bool UNIQUE_NAME = true;         \
  if (UNIQUE_NAME)                        \
  {                                   \
                       \
    UNIQUE_NAME = false;                  \
  }                                   \
}
#define ASSERT(expr, ...) { if(!(expr)) { DEBUG_BREAK("", __VA_ARGS__); } }