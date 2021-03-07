#pragma once
#include <vulkan/vulkan.h>

struct RenderContext
{
  VkCommandBuffer m_CommandBuffer;
  size_t m_CommandBufferIdx;
  size_t m_ImageIdx;
};

//TODO: I guess it could be called a pass instead?
class IRenderable
{
public:
  virtual void Init() = 0;
  virtual void Update(RenderContext& ctx) = 0;
  virtual void Render(RenderContext& ctx) = 0;
  virtual void Cleanup() = 0;
};