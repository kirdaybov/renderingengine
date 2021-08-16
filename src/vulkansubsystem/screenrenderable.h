#pragma once
#include "renderable.h"
#include <vector>
#include <vulkansubsystem/buffer.h>
#include "vertexdescription.h"

// Renders two triangles to cover the whole screen
class ScreenRenderable : public IRenderable
{
  VkDescriptorSetLayout m_DescriptorSetLayout;
  VkPipelineLayout m_PipelineLayout;
  VkPipeline m_GraphicsPipeline;
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  std::vector<Vertex> m_Vertices;
  std::vector<uint32_t> m_Indices;
  Buffer* m_VertexBuffer;
  Buffer* m_IndexBuffer;
  void LoadModelFBX();

  //
  void CreateDescriptorSetLayout();
  Buffer* m_UniformBuffer;
  void CreateUniformBuffer();
  void UpdateUniformBuffer();

  //
  VkDescriptorPool m_DescriptorPool;
  std::vector<VkDescriptorSet> m_DescriptorSets;
  void CreateDescriptorPool();
  void CreateDescriptorSets();

  //
  void CreateImage(const char* name, VkImage& image, VkDeviceMemory& memory);
  void CreateTextureImage();

  //VkImage m_TextureImage;
  //VkDeviceMemory m_TextureImageMemory;
  //VkImageView m_TextureImageView;

  void CreateTextureImageView();

  //
  //VkSampler m_TextureSampler;
  void CreateTextureSampler();


  void Init() override;
  void Update(RenderContext& ctx) override;
  void Render(RenderContext& ctx) override;
  void OnCleanupSwapChain() override;
  void CreateGraphicsPipeline() override;
  void Cleanup() override;

  const char* GetName() const override { return "Screen"; }
};