#pragma once
#include <vulkan/vulkan.h>

//TODO: I guess it could be called a pass instead?
class IRenderable
{
public:
  virtual void Init() = 0;
  virtual void Update() = 0;
  virtual void Render(VkCommandBuffer commandBuffer) = 0;
  virtual void Cleanup() = 0;
};