#pragma once
#include "renderable.h"
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <vulkansubsystem/buffer.h>

struct Vertex
{
  glm::vec3 m_Position;
  glm::vec3 m_Normal;
  glm::vec2 m_TexCoord;
  glm::vec3 m_Tangent;
  glm::vec3 m_Bitangent;

  static VkVertexInputBindingDescription GetBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 5> GetAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, m_Position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, m_Normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, m_TexCoord);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, m_Tangent);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, m_Bitangent);

    return attributeDescriptions;
  }
};

class MeshRenderable : public IRenderable
{
  VkDescriptorSetLayout m_DescriptorSetLayout;
  VkPipelineLayout m_PipelineLayout;
  VkPipeline m_GraphicsPipeline;
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  std::vector<Vertex> m_Vertices;
  std::vector<uint32_t> m_Indices;
  Buffer m_VertexBuffer;
  Buffer m_IndexBuffer;
  void LoadModelFBX();

  //
  void CreateDescriptorSetLayout();
  Buffer m_UniformBuffer;
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

  VkImage m_TextureImage;
  VkDeviceMemory m_TextureImageMemory;
  VkImageView m_TextureImageView;

  VkImage m_NormalImage;
  VkDeviceMemory m_NormalImageMemory;
  VkImageView m_NormalImageView;

  void CreateTextureImageView();

  //
  VkSampler m_TextureSampler;
  VkSampler m_NormalSampler;
  void CreateTextureSampler();


  void Init() override;
  void Update(RenderContext& ctx) override;
  void Render(RenderContext& ctx) override;
  void OnCleanupSwapChain() override;
  void CreateGraphicsPipeline() override;
  void Cleanup() override;
};