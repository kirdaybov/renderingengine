#pragma once

#include <cstdio>
#include "Windows.h"

#define UNIQUE_NAME __FILE__##__LINE__
#define LOGF(fmt, ...)                                    \
{                                                         \
  char buff[1024];                                        \
  sprintf(buff, "\n" fmt, __VA_ARGS__);                   \
  OutputDebugString(buff);                                \
}
#define DEBUG_BREAK(fmt, ...)                                 \
{                                                             \
  LOGF("%s %d: " fmt, __FILE__, __LINE__, __VA_ARGS__);          \
  static bool UNIQUE_NAME = true;         \
  if (UNIQUE_NAME)                        \
  {                                   \
    if (IsDebuggerPresent()) __debugbreak();                   \
    UNIQUE_NAME = false;                  \
  }                                   \
}
#define ASSERT(expr, ...) { if(!(expr)) { DEBUG_BREAK("", __VA_ARGS__); } }