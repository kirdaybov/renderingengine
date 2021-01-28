#pragma once

#include <cstdio>

#define LOGF(fmt, ...) printf("\n" fmt, __VA_ARGS__)
#define DEBUG_BREAK(fmt, ...)                                 \
{                                                             \
  LOGF("%s %d: " fmt, __FILE__, __LINE__, __VA_ARGS__);          \
  __debugbreak();                                             \
}
#define ASSERT(expr, ...) { if(!(expr)) { DEBUG_BREAK("", __VA_ARGS__); } }