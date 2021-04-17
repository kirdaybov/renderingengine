#pragma once
#include "renderable.h"
#include <glm/glm.hpp>
#include <array>
#include <vector>

struct Vertex
{
  glm::vec3 m_Position;
  glm::vec2 m_TexCoord;
  
  static VkVertexInputBindingDescription GetBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, m_Position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, m_TexCoord);

    return attributeDescriptions;
  }
};

class MeshRenderable : public IRenderable
{
  void CreateGraphicsPipeline();
  VkDescriptorSetLayout m_DescriptorSetLayout;
  VkPipelineLayout m_PipelineLayout;
  VkPipeline m_GraphicsPipeline;
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  std::vector<Vertex> m_Vertices;
  std::vector<uint32_t> m_Indices;
  VkBuffer m_VertexBuffer;
  VkDeviceMemory m_VertexBufferMemory;
  VkBuffer m_IndexBuffer;
  VkDeviceMemory m_IndexBufferMemory;
  void LoadModelTinyObj();
  void LoadModelFBX();

  //
  void CreateDescriptorSetLayout();
  std::vector<VkBuffer> m_UniformBuffers;
  std::vector<VkDeviceMemory> m_UniformBuffersMemory;
  void CreateUniformBuffers();
  void UpdateUniformBuffer(uint32_t currentImage);

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

  void CreateTextureImageView();

  //
  VkSampler m_TextureSampler;
  void CreateTextureSampler();


  void Init() override;
  void Update(RenderContext& ctx) override;
  void Render(RenderContext& ctx) override;
  void Cleanup() override;
};