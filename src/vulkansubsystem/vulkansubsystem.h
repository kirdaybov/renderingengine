#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkancommon.h"
#include <vector>
#include <array>
#include <Windows.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderable.h"
#include "buffer.h"

struct GLFWwindow;

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

class Renderer
{
public:
  void Init(GLFWwindow* window);
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
    "VK_LAYER_KHRONOS_validation"
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

  std::vector<IRenderable*> m_Renderables;

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
  void CreateRenderPass();
  VkRenderPass m_RenderPass;

  //
  std::vector<VkFramebuffer> m_SwapChainFramebuffers;
  void CreateFramebuffers();

  //
  VkCommandPool m_CommandPool;
  void CreateCommandPool();
  std::vector<VkCommandBuffer> m_CommandBuffers;
  void CreateCommandBuffers(uint32_t imageIdx);
  void FreeCommandBuffers();
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

public:
  //
  void DrawFrame();
  void DeviceWaitIdle() { vkDeviceWaitIdle(m_Device); }

  void ScheduleShaderUpdate() { m_ShaderUpdateScheduled = true; }
private:
  bool m_ShaderUpdateScheduled = false;
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
  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  
  //
  VkImage m_DepthImage;
  VkDeviceMemory m_DepthImageMemory;
  VkImageView m_DepthImageView;
  void CreateDepthResources();
  VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  VkFormat FindDepthFormat();
  bool HasStencilComponent(VkFormat format);

//TODO: maybe it shouldn't be public?
public:
  VkShaderModule CreateShaderModule(char* code, int size);
  void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer);
  void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
  void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
  VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
  void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
  void CopyBufferToImage(Buffer& buffer, VkImage image, uint32_t width, uint32_t height);
  void CopyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);

  VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; }
  size_t GetSwapChainImagesCount() { return m_SwapChainImages.size(); }
  VkDevice GetDevice() { return m_Device; }
  VkRenderPass GetRenderPass() { return m_RenderPass; }
};
