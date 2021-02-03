#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <Windows.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

struct GLFWwindow;

#define VK_CHECK(code)                             \
{                                                  \
  VkResult result = code;                          \
  if (result != VK_SUCCESS)                        \
  {                                                \
    DEBUG_BREAK("Vulkan error: %d", result);       \
  }                                                \
}

struct Vertex
{
  glm::vec3 m_Position;
  glm::vec3 m_Color;
  glm::vec2 m_TexCoord;

  static VkVertexInputBindingDescription GetBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, m_Position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, m_Color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, m_TexCoord);

    return attributeDescriptions;
  }
};

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

struct QueueFamilyIndices
{
  int graphicsFamily = -1;
  int presentFamily = -1;


  bool IsComplete() const
  {
    return 
      graphicsFamily > -1
      && presentFamily > -1
      ;
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSubsystem
{
public:
  void InitVulkan(GLFWwindow* window);
  void Cleanup();
private:

  GLFWwindow* m_Window = nullptr;
  //
  void CreateInstance();
  VkInstance m_Instance;

  //
  std::vector<const char*> GetRequiredExtensions() const;
  void ListAvailableExtensions() const;
  
  //
  void ListAvailableValidationLayers() const;
  bool CheckValidationLayerSupport() const;
  const std::vector<const char*> m_ValidationLayers =
  {
    "VK_LAYER_LUNARG_standard_validation"
  };
#ifdef NDEBUG
  const bool m_EnableValidationLayers = false;
#else
  const bool m_EnableValidationLayers = true;
#endif //#ifdef NDEBUG
  
  //
  void SetupDebugMessenger();
  VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* createInfo, 
    const VkAllocationCallbacks* allocator, 
    VkDebugUtilsMessengerEXT* debugMessenger
  );
  void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* allocator
  );
  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData
  );
  VkDebugUtilsMessengerEXT m_DebugMessenger;

  //
  VkSurfaceKHR m_Surface;
  void CreateSurface(GLFWwindow* window);

  //
  void PickPhysicalDevice();
  bool IsDeviceSuitable(VkPhysicalDevice device) const;
  VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
  const std::vector<const char*> m_DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  //
  QueueFamilyIndices m_FamilyIndices;
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;

  //
  VkDevice m_Device;
  void CreateLogicalDevice();
  
  //
  VkQueue m_GraphicsQueue;

  //
  VkQueue m_PresentQueue;

  //
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
  void CreateSwapChain();
  VkSwapchainKHR m_SwapChain;
  std::vector<VkImage> m_SwapChainImages;
  VkFormat m_SwapChainImageFormat;
  VkExtent2D m_SwapChainExtent;
  void RecreateSwapChain();
  void CleanupSwapChain(); // TODO: update swap chain on resize

  //
  void CreateImageViews();
  std::vector<VkImageView> m_SwapChainImageViews;

  //
  void CreateGraphicsPipeline();
  VkShaderModule CreateShaderModule(const std::vector<char>& code);
  VkRenderPass m_RenderPass;
  VkDescriptorSetLayout m_DescriptorSetLayout;
  VkPipelineLayout m_PipelineLayout;
  void CreateRenderPass();
  VkPipeline m_GraphicsPipeline;

  // ImGui
  void CreateImGuiPipeline();
  VkPipeline m_ImGuiPipeline;
  VkPipelineLayout m_ImGuiPipelineLayout;
  VkDescriptorSetLayout m_ImGuiDescriptorSetLayout;
  VkDescriptorSet m_ImGuiDescriptorSet;
  void CreateImGuiBuffers();
  void UpdateImGuiBuffers(bool unmap = true);
  VkBuffer m_ImGuiVertexBuffer;
  VkBuffer m_ImGuiIndexBuffer;
  VkDeviceMemory m_ImGuiVertexBufferMemory;
  VkDeviceMemory m_ImGuiIndexBufferMemory;

  VkDeviceMemory m_ImGuiFontMemory;
  VkImage m_ImGuiFontImage;
  VkImageView m_ImGuiFontView;
  VkSampler m_ImGuiFontSampler;


  struct ImGuiConst
  {
    glm::vec2 scale;
    glm::vec2 translate;
  } m_ImGuiConst;

  //
  std::vector<VkFramebuffer> m_SwapChainFramebuffers;
  void CreateFramebuffers();

  //
  VkCommandPool m_CommandPool;
  void CreateCommandPool();
  std::vector<VkCommandBuffer> m_CommandBuffers;
  void CreateCommandBuffers();
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

public:
  //
  void DrawFrame();
  void DeviceWaitIdle() { vkDeviceWaitIdle(m_Device); }
private:
  enum
  {
    MaxFramesInFlight = 1
  };
  size_t m_TotalFrame = 0;
  size_t m_CurrentFrame = 0;
  std::vector<VkSemaphore> m_ImageAvailableSemaphores;
  std::vector<VkSemaphore> m_RenderFinishedSemaphores;
  std::vector<VkFence> m_InFlightFences;
  void CreateSyncObjects();

  //
  void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  std::vector<Vertex> m_Vertices;
  std::vector<uint32_t> m_Indices;
  VkBuffer m_VertexBuffer;
  VkDeviceMemory m_VertexBufferMemory;
  VkBuffer m_IndexBuffer;
  VkDeviceMemory m_IndexBufferMemory;
  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
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
  void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
  VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
  void CreateTextureImage();
  VkImage m_TextureImage;
  VkDeviceMemory m_TextureImageMemory;
  VkImageView m_TextureImageView;
  void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  void CreateTextureImageView();

  //
  VkSampler m_TextureSampler;
  void CreateTextureSampler();

  //
  VkImage m_DepthImage;
  VkDeviceMemory m_DepthImageMemory;
  VkImageView m_DepthImageView;
  void CreateDepthResources();
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  VkFormat FindDepthFormat();
  bool HasStencilComponent(VkFormat format);
};