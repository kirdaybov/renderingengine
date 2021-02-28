#pragma once

#include <vulkan/vulkan.h>

#define VK_CHECK(code)                             \
{                                                  \
  VkResult result = code;                          \
  if (result != VK_SUCCESS)                        \
  {                                                \
    DEBUG_BREAK("Vulkan error: %d", result);       \
  }                                                \
}