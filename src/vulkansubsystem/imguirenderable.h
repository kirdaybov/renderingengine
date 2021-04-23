#pragma once 

#include "renderable.h"
#include <imgui.h>
#include "vulkancommon.h"
#include <glm/vec2.hpp>
#include <array>
#include "buffer.h"

struct ImGuiVertexDescriptor
{
  static VkVertexInputBindingDescription GetBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ImDrawVert);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

    return attributeDescriptions;
  }
};

class ImGuiRenderable : public IRenderable
{
  void CreateImGuiPipeline();
  void CreateDescriptorSets();
  void CreateImGuiBuffers();
  void UpdateImGuiBuffers(bool unmap = true);

  VkPipeline m_ImGuiPipeline;
  VkPipelineLayout m_ImGuiPipelineLayout;
  VkDescriptorPool m_ImGuiDescriptorPool;
  VkDescriptorSetLayout m_ImGuiDescriptorSetLayout;
  VkDescriptorSet m_ImGuiDescriptorSet;
  Buffer m_ImGuiVertexBuffer;
  Buffer m_ImGuiIndexBuffer;
  
  VkDeviceMemory m_ImGuiFontMemory;
  VkImage m_ImGuiFontImage;
  VkImageView m_ImGuiFontView;
  VkSampler m_ImGuiFontSampler;


  struct ImGuiConst
  {
    glm::vec2 scale;
    glm::vec2 translate;
  } m_ImGuiConst;

  void Init() override;
  void Update(RenderContext& ctx) override;
  void Render(RenderContext& ctx) override;
  void Cleanup() override;
};
