#pragma once
#include <vulkan/vulkan.h>

struct RenderContext
{
  VkCommandBuffer m_CommandBuffer;
  size_t m_CommandBufferIdx;
  size_t m_ImageIdx;
};

class IRenderable
{
public:
  virtual ~IRenderable() {}

  virtual void Init() = 0;
  virtual void Update(RenderContext& ctx) = 0;
  virtual void Render(RenderContext& ctx) = 0;
  virtual void OnCleanupSwapChain() = 0;
  virtual void CreateGraphicsPipeline() = 0;
  virtual void Cleanup() = 0;

  // For ImGui
  bool& GetShow() { return m_Show; }
  virtual const char* GetName() const = 0;

private:
  bool m_Show = true;
};