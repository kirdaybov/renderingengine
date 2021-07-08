#pragma once
#include <glm/glm.hpp>
#include <array>
#include <vulkan/vulkan_core.h>

struct Vertex
{
  glm::vec3 m_Position;
  glm::vec2 m_TexCoord;

  Vertex() {}
  Vertex(const glm::vec3& position) : m_Position(position) {}
  
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