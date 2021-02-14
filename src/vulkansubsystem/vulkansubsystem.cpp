#include "vulkansubsystem.h"
#include "config.h"
#include "logger/logger.h"

#include <vector>
#include <set>
#include <algorithm>

// TODO: remove
#include <fstream>

#include <chrono>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "assetmanager/assetmanager.h"

#include <fbxsdk.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "app/application.h"
#include "gamestate/gamestate.h"

#include "imgui.h"

const std::string MODEL_PATH = "C:/_work/tbs/models/chalet.obj";
const std::string TEXTURE_PATH = "C:/_work/tbs/textures/chalet.jpg";

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

void VulkanSubsystem::InitVulkan(GLFWwindow* window)
{
  CreateInstance();
  if (m_EnableValidationLayers)
  {
    SetupDebugMessenger();
  }
  CreateSurface(window);
  m_Window = window;
  PickPhysicalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateDescriptorSetLayout();
  CreateGraphicsPipeline();
  CreateImGuiPipeline();
  CreateCommandPool();
  CreateDepthResources();
  CreateFramebuffers();
  CreateTextureImage();
  CreateTextureImageView();
  CreateTextureSampler();
  LoadModelFBX();
  CreateVertexBuffer();
  CreateIndexBuffer();
  CreateImGuiBuffers();
  UpdateImGuiBuffers(false);
  CreateUniformBuffers();
  CreateDescriptorPool();
  CreateDescriptorSets();
  //CreateCommandBuffers();
  CreateSyncObjects();
}

void VulkanSubsystem::Cleanup()
{
  CleanupSwapChain();

  vkDestroySampler(m_Device, m_TextureSampler, nullptr);
  vkDestroyImageView(m_Device, m_TextureImageView, nullptr);
  vkDestroyImage(m_Device, m_TextureImage, nullptr);
  vkFreeMemory(m_Device, m_TextureImageMemory, nullptr);

  vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
  vkDestroyDescriptorSetLayout(m_Device, m_ImGuiDescriptorSetLayout, nullptr);

  for (size_t i = 0; i < m_SwapChainImages.size(); i++)
  {
    vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
    vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
  }

  vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
  vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);
  vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
  vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

  for (size_t i = 0; i < MaxFramesInFlight; i++)
  {
    vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
    vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
  
  vkDestroyDevice(m_Device, nullptr);
  if (m_EnableValidationLayers)
    DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
  vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
  vkDestroyInstance(m_Instance, nullptr);
}

void VulkanSubsystem::CleanupSwapChain()
{
  vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
  vkDestroyImage(m_Device, m_DepthImage, nullptr);
  vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);

  for (auto framebuffer : m_SwapChainFramebuffers)
  {
    vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
  }
  vkFreeCommandBuffers(m_Device, m_CommandPool, static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
  vkDestroyPipeline(m_Device, m_ImGuiPipeline, nullptr);
  vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
  vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
  vkDestroyPipelineLayout(m_Device, m_ImGuiPipelineLayout, nullptr);
  vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
  for (auto& imageView : m_SwapChainImageViews)
  {
    vkDestroyImageView(m_Device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

void VulkanSubsystem::RecreateSwapChain()
{
  vkDeviceWaitIdle(m_Device);

  CleanupSwapChain();

  CreateSwapChain();
  CreateImageViews();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateImGuiPipeline();
  CreateDepthResources();
  CreateFramebuffers();
  //CreateCommandBuffers();
}

void VulkanSubsystem::CreateInstance()
{
  //ListAvailableExtensions();
  //ListAvailableValidationLayers();

  if (m_EnableValidationLayers && !CheckValidationLayerSupport())
  {
    DEBUG_BREAK("VK error: validation layers are not supported");
  }

  VkApplicationInfo applicationInfo = {};
  applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  applicationInfo.pApplicationName = Consts::ms_ProjectName;
  applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.pEngineName = Consts::ms_EngineName;
  applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &applicationInfo;
  auto extensions = GetRequiredExtensions();
  instanceCreateInfo.enabledExtensionCount = extensions.size();
  instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

  if (m_EnableValidationLayers)
  {
    instanceCreateInfo.enabledLayerCount = m_ValidationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
  }
  else
  {
    instanceCreateInfo.enabledLayerCount = 0;
  }

  VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance));
}

void VulkanSubsystem::ListAvailableExtensions() const
{
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  for (auto& extension : extensions)
    LOGF("Extension: %s", extension.extensionName);
}

void VulkanSubsystem::ListAvailableValidationLayers() const
{
  uint32_t validationLayerCount = 0;
  vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(validationLayerCount);
  vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());
  for (auto& layer : availableLayers)
    LOGF("Layer: %s", layer.layerName);
}

bool VulkanSubsystem::CheckValidationLayerSupport() const
{
  uint32_t validationLayerCount = 0;
  vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(validationLayerCount);
  vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

  bool supported = true;
  for (auto& validationLayer : m_ValidationLayers)
  {
    bool layerFound = false;
    for (auto& availableLayer : availableLayers)
    {
      if (strcmp(validationLayer, availableLayer.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }
    if (!layerFound)
    {
      LOGF("Layer is not supported: %s", validationLayer);
      supported = false;
    }
  }
  
  return supported;
}

std::vector<const char*> VulkanSubsystem::GetRequiredExtensions() const
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (m_EnableValidationLayers)
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions.push_back("VK_KHR_surface");
  extensions.push_back("VK_KHR_win32_surface");
  return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanSubsystem::DebugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
  void* userData
)
{
  DEBUG_BREAK("Validation layer: %s", callbackData->pMessage);
  return VK_FALSE;
}

void VulkanSubsystem::SetupDebugMessenger()
{
  VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = DebugCallback;
  createInfo.pUserData = nullptr;

  if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
    DEBUG_BREAK("Vk error: couldn't create debug messenger");
}

VkResult VulkanSubsystem::CreateDebugUtilsMessengerEXT(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
  const VkAllocationCallbacks* allocator,
  VkDebugUtilsMessengerEXT* debugMessenger
)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    return func(instance, createInfo, allocator, debugMessenger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void VulkanSubsystem::DestroyDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerEXT debugMessenger,
  const VkAllocationCallbacks* allocator
)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, allocator);
  }
}

void VulkanSubsystem::CreateSurface(GLFWwindow* window)
{
  VkWin32SurfaceCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hwnd = glfwGetWin32Window(window);
  createInfo.hinstance = GetModuleHandle(nullptr);
  if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS)
    DEBUG_BREAK("Vk error: couldn't create surface");
}

void VulkanSubsystem::PickPhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
  for (auto& device : devices)
  {
    if (IsDeviceSuitable(device))
    {
      m_PhysicalDevice = device;
      break;
    }
  }
  if (m_PhysicalDevice == VK_NULL_HANDLE)
    DEBUG_BREAK("Vk error: couldn't find suitable device");
}

bool VulkanSubsystem::IsDeviceSuitable(VkPhysicalDevice device) const
{
  bool extensionsSupported = CheckDeviceExtensionSupport(device);
  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
    SwapChainSupportDetails details = QuerySwapChainSupport(device);
    swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
  return FindQueueFamilies(device).IsComplete() && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanSubsystem::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

QueueFamilyIndices VulkanSubsystem::FindQueueFamilies(VkPhysicalDevice device) const
{
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  int index = 0;
  for (auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphicsFamily = index;
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, index, m_Surface, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport)
      indices.presentFamily = index;
    ++index;
  }

  return indices;
}

void VulkanSubsystem::CreateLogicalDevice()
{
  m_FamilyIndices = FindQueueFamilies(m_PhysicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<int> uniqueQueueFamilies = 
  {
    m_FamilyIndices.graphicsFamily, 
    m_FamilyIndices.presentFamily
  };
  for (int queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = queueCreateInfos.size();
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = m_DeviceExtensions.size();
  createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();
  if (m_EnableValidationLayers)
  {
    createInfo.enabledLayerCount = m_ValidationLayers.size();
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
    DEBUG_BREAK("Vk error: couldn't create logical device");

  vkGetDeviceQueue(m_Device, m_FamilyIndices.graphicsFamily, 0, &m_GraphicsQueue);
  vkGetDeviceQueue(m_Device, m_FamilyIndices.presentFamily, 0, &m_PresentQueue);
}

SwapChainSupportDetails VulkanSubsystem::QuerySwapChainSupport(VkPhysicalDevice device) const
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR VulkanSubsystem::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
  SwapChainSupportDetails details = QuerySwapChainSupport(m_PhysicalDevice);
  if (details.formats.size() == 1 && details.formats[0].format == VK_FORMAT_UNDEFINED)
  {
    return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  }
  for (const auto& format : details.formats)
  {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  return details.formats[0];
}

VkPresentModeKHR VulkanSubsystem::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
  VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

  for (const auto& availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      return availablePresentMode;
    }
    else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
    {
      bestMode = availablePresentMode;
    }
  }

  return bestMode;
}

VkExtent2D VulkanSubsystem::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
  if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
  {
    return capabilities.currentExtent;
  }
  else
  {
    const int width = 1024;
    const int height = 768;
    VkExtent2D actualExtent = { width, height };

    actualExtent.width = (std::max)(capabilities.minImageExtent.width, (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = (std::max)(capabilities.minImageExtent.height, (std::min)(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

void VulkanSubsystem::CreateSwapChain()
{
  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(extent.width, extent.height);
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_Surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
  uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
    DEBUG_BREAK("Vk error: couldn't create swap chain");

  vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
  m_SwapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

  m_SwapChainImageFormat = surfaceFormat.format;
  m_SwapChainExtent = extent;
}

void VulkanSubsystem::CreateImageViews()
{
  m_SwapChainImageViews.resize(m_SwapChainImages.size());

  for (int i = 0; i < m_SwapChainImages.size(); i++)
  {
    m_SwapChainImageViews[i] = CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

std::vector<char> ReadFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

void VulkanSubsystem::CreateGraphicsPipeline()
{
  ShaderBinary* vsShader = gApplicationInstanceManager.GetInstance(m_Window).GetShaderCompiler().GetShader("meshvs");
  ShaderBinary* psShader = gApplicationInstanceManager.GetInstance(m_Window).GetShaderCompiler().GetShader("meshps");

  VkShaderModule vertShaderModule = CreateShaderModule(vsShader->GetData(), vsShader->GetSize());
  VkShaderModule fragShaderModule = CreateShaderModule(psShader->GetData(), psShader->GetSize());

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

  auto bindingDescription = Vertex::GetBindingDescription();
  auto attributeDescription = Vertex::GetAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)m_SwapChainExtent.width;
  viewport.height = (float)m_SwapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = { 0, 0 };
  scissor.extent = m_SwapChainExtent;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1; // Optional
  pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout; // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
  pipelineLayoutInfo.pNext = nullptr;
  pipelineLayoutInfo.flags = 0;

  if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f; // Optional
  depthStencil.maxDepthBounds = 1.0f; // Optional
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {}; // Optional
  depthStencil.back = {}; // Optional

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil; // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = nullptr; // Optional
  pipelineInfo.layout = m_PipelineLayout;
  pipelineInfo.renderPass = m_RenderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
  vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
}

void VulkanSubsystem::CreateImGuiPipeline()
{
  ShaderBinary* vsShader = gApplicationInstanceManager.GetInstance(m_Window).GetShaderCompiler().GetShader("uivs");
  ShaderBinary* psShader = gApplicationInstanceManager.GetInstance(m_Window).GetShaderCompiler().GetShader("uips");

  VkShaderModule vertShaderModule = CreateShaderModule(vsShader->GetData(), vsShader->GetSize());
  VkShaderModule fragShaderModule = CreateShaderModule(psShader->GetData(), psShader->GetSize());

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

  auto bindingDescription = ImGuiVertexDescriptor::GetBindingDescription();
  auto attributeDescription = ImGuiVertexDescriptor::GetAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.vertexAttributeDescriptionCount = attributeDescription.size();
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)m_SwapChainExtent.width;
  viewport.height = (float)m_SwapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = { 0, 0 };
  scissor.extent = m_SwapChainExtent;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE; // ?
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE; // ?
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  // multisampling.sampleShadingEnable = VK_FALSE; // ?
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable = VK_FALSE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  //depthStencil.depthBoundsTestEnable = VK_FALSE;
  //depthStencil.minDepthBounds = 0.0f; // Optional
  //depthStencil.maxDepthBounds = 1.0f; // Optional
  //depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = depthStencil.back; // Optional
  depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS; // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  //colorBlending.logicOpEnable = VK_FALSE;
  //colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  //colorBlending.blendConstants[0] = 0.0f; // Optional
  //colorBlending.blendConstants[1] = 0.0f; // Optional
  //colorBlending.blendConstants[2] = 0.0f; // Optional
  //colorBlending.blendConstants[3] = 0.0f; // Optional

  VkDynamicState dynamicStateEnables[2] = 
  { 
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo;
  pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables;
  pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
  pipelineDynamicStateCreateInfo.pNext = nullptr;
  pipelineDynamicStateCreateInfo.flags = 0;

  VkPushConstantRange pushConstantRange;
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(ImGuiConst);

  VkDescriptorSetLayoutBinding setLayoutBinding;
  setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  setLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  setLayoutBinding.binding = 0;
  setLayoutBinding.descriptorCount = 1;

  VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
      setLayoutBinding,
      samplerLayoutBinding
  };

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
  descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
  descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

  if (vkCreateDescriptorSetLayout(m_Device, &descriptorSetLayoutCreateInfo, nullptr, &m_ImGuiDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.setLayoutCount = 1;
  pipelineLayoutCreateInfo.pSetLayouts = &m_ImGuiDescriptorSetLayout;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
  pipelineLayoutCreateInfo.pNext = nullptr;
  pipelineLayoutCreateInfo.flags = 0;

  if (vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &m_ImGuiPipelineLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
  pipelineInfo.layout = m_ImGuiPipelineLayout;
  pipelineInfo.renderPass = m_RenderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_ImGuiPipeline) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
  vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
}

VkShaderModule VulkanSubsystem::CreateShaderModule(char* code, int size)
{
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = size;
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

void VulkanSubsystem::CreateRenderPass()
{
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = m_SwapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = FindDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef = {};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create render pass!");
  }
}

void VulkanSubsystem::CreateFramebuffers()
{
  m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

  for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
  {
    std::array<VkImageView, 2> attachments = {
      m_SwapChainImageViews[i],
      m_DepthImageView
    };

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_RenderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_SwapChainExtent.width;
    framebufferInfo.height = m_SwapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void VulkanSubsystem::CreateCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);
  
  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
  poolInfo.flags = 0; // Optional

  if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void VulkanSubsystem::CreateImGuiBuffers()
{
  ImGuiIO& io = ImGui::GetIO();

  // Create font texture
  unsigned char* fontData;
  int texWidth, texHeight;
  io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
  VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
  

  // Create target image for copy
  //VkImageCreateInfo imageInfo = {};
  //imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  //imageInfo.imageType = VK_IMAGE_TYPE_2D;
  //imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  //imageInfo.extent.width = texWidth;
  //imageInfo.extent.height = texHeight;
  //imageInfo.extent.depth = 1;
  //imageInfo.mipLevels = 1;
  //imageInfo.arrayLayers = 1;
  //imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  //imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  //imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  //imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  //imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  //VK_CHECK(vkCreateImage(m_Device, &imageInfo, nullptr, &m_ImGuiFontImage));
  //VkMemoryRequirements memReqs;
  //vkGetImageMemoryRequirements(m_Device, m_ImGuiFontImage, &memReqs);
  //VkMemoryAllocateInfo memAllocInfo = {};
  //memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //memAllocInfo.allocationSize = memReqs.size;
  //memAllocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  //VK_CHECK(vkAllocateMemory(m_Device, &memAllocInfo, nullptr, &m_ImGuiFontMemory));
  //VK_CHECK(vkBindImageMemory(m_Device, m_ImGuiFontImage, m_ImGuiFontMemory, 0));
  //
  
  // Staging buffers for font data upload
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  CreateBuffer(uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(m_Device, stagingBufferMemory, 0, uploadSize, 0, &data);
  memcpy(data, fontData, uploadSize);
  vkUnmapMemory(m_Device ,stagingBufferMemory);

  CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ImGuiFontImage, m_ImGuiFontMemory);
  TransitionImageLayout(m_ImGuiFontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(stagingBuffer, m_ImGuiFontImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  TransitionImageLayout(m_ImGuiFontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // Image view
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = m_ImGuiFontImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.layerCount = 1;
  VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImGuiFontView));

  //void* data;
  //vkMapMemory(m_Device, stagingBufferMemory, 0, uploadSize, 0, &data);
  //memcpy(data, fontData, uploadSize);
  //vkUnmapMemory(m_Device ,stagingBufferMemory);
  //
  //TransitionImageLayout(m_ImGuiFontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  //
  //// Copy buffer data to font image
  //VkCommandBuffer copyCmd = {};
  //VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
  //commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  //commandBufferAllocateInfo.commandPool = m_CommandPool;
  //commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  //commandBufferAllocateInfo.commandBufferCount = 1;
  //VK_CHECK(vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, &copyCmd));
  //VkCommandBufferBeginInfo cmdBufInfo = {};
  //cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  //VK_CHECK(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
  //
  //// Prepare for transfer
  //VkImageSubresourceRange subresourceRange = {};
  //subresourceRange.aspectMask = aspectMask;
  //subresourceRange.baseMipLevel = 0;
  //subresourceRange.levelCount = 1;
  //subresourceRange.layerCount = 1;
  //vks::tools::setImageLayout(
  //  copyCmd,
  //  fontImage,
  //  VK_IMAGE_ASPECT_COLOR_BIT,
  //  VK_IMAGE_LAYOUT_UNDEFINED,
  //  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  //  VK_PIPELINE_STAGE_HOST_BIT,
  //  VK_PIPELINE_STAGE_TRANSFER_BIT);
  //
  //// Copy
  //VkBufferImageCopy bufferCopyRegion = {};
  //bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  //bufferCopyRegion.imageSubresource.layerCount = 1;
  //bufferCopyRegion.imageExtent.width = texWidth;
  //bufferCopyRegion.imageExtent.height = texHeight;
  //bufferCopyRegion.imageExtent.depth = 1;
  //
  //vkCmdCopyBufferToImage(
  //  copyCmd,
  //  stagingBuffer.buffer,
  //  fontImage,
  //  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  //  1,
  //  &bufferCopyRegion
  //);
  //
  //// Prepare for shader read
  //vks::tools::setImageLayout(
  //  copyCmd,
  //  fontImage,
  //  VK_IMAGE_ASPECT_COLOR_BIT,
  //  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  //  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  //  VK_PIPELINE_STAGE_TRANSFER_BIT,
  //  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  //
  //device->flushCommandBuffer(copyCmd, copyQueue, true);

  vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
  vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

  // Font texture Sampler
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_ImGuiFontSampler));
}

void VulkanSubsystem::UpdateImGuiBuffers(bool unmap)
{
  ImDrawData* imDrawData = ImGui::GetDrawData();
  if (!imDrawData)
  {
    return;
  }

  // Note: Alignment is done inside buffer creation
  VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
  VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

  if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
    return;
  }

  // Update buffers only if vertex or index count has been changed compared to current buffer size

  // Vertex buffer
  //if ((m_ImGuiVertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount))
  void* vertexData;
  void* indexData;
  
  {
    if (m_ImGuiVertexBufferMemory)
    {
      vkUnmapMemory(m_Device, m_ImGuiVertexBufferMemory);
    }
    if (m_ImGuiVertexBuffer)
    {
      vkDestroyBuffer(m_Device, m_ImGuiVertexBuffer, nullptr);
    }
    if (m_ImGuiVertexBufferMemory)
    {
      vkFreeMemory(m_Device, m_ImGuiVertexBufferMemory, nullptr);
    }
    CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_ImGuiVertexBuffer, m_ImGuiVertexBufferMemory);
    //VK_CHECK_RESULT(device->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vertexBuffer, vertexBufferSize));
    //vertexCount = imDrawData->TotalVtxCount;
    //vertexBuffer.unmap();
    vkMapMemory(m_Device, m_ImGuiVertexBufferMemory, 0, VK_WHOLE_SIZE, 0, &vertexData);
  }

  // Index buffer
  //VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
  //if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount))
  {
    if (m_ImGuiIndexBufferMemory)
    {
      vkUnmapMemory(m_Device, m_ImGuiIndexBufferMemory);
    }
    if (m_ImGuiIndexBuffer)
    {
      vkDestroyBuffer(m_Device, m_ImGuiIndexBuffer, nullptr);
    }
    if (m_ImGuiIndexBufferMemory)
    {
      vkFreeMemory(m_Device, m_ImGuiIndexBufferMemory, nullptr);
    }
    CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_ImGuiIndexBuffer, m_ImGuiIndexBufferMemory);

    vkMapMemory(m_Device, m_ImGuiIndexBufferMemory, 0, VK_WHOLE_SIZE, 0, &indexData);
  }

  // Upload data
  ImDrawVert* vtxDst = (ImDrawVert*)vertexData;
  ImDrawIdx* idxDst = (ImDrawIdx*)indexData;

  for (int n = 0; n < imDrawData->CmdListsCount; n++) {
    const ImDrawList* cmd_list = imDrawData->CmdLists[n];
    memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtxDst += cmd_list->VtxBuffer.Size;
    idxDst += cmd_list->IdxBuffer.Size;
  }

  // Flush to make writes visible to GPU
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = m_ImGuiVertexBufferMemory;
  mappedRange.offset = 0;
  mappedRange.size = VK_WHOLE_SIZE;
  vkFlushMappedMemoryRanges(m_Device, 1, &mappedRange);

  //VkMappedMemoryRange mappedRange;
  //mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = m_ImGuiIndexBufferMemory;
  mappedRange.offset = 0;
  mappedRange.size = VK_WHOLE_SIZE;
  vkFlushMappedMemoryRanges(m_Device, 1, &mappedRange);
}

void VulkanSubsystem::FreeCommandBuffers()
{
  if (m_CommandBuffers.size() > 0)
  {
    vkFreeCommandBuffers(m_Device, m_CommandPool, m_CommandBuffers.size(), m_CommandBuffers.data());
  }
}

void VulkanSubsystem::CreateCommandBuffers()
{
  m_CommandBuffers.resize(m_SwapChainFramebuffers.size());

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_CommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

  if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  UpdateImGuiBuffers(false);

  for (size_t b = 0; b < m_CommandBuffers.size(); b++)
  {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(m_CommandBuffers[b], &beginInfo) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapChainFramebuffers[b];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_SwapChainExtent;


    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_CommandBuffers[b], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_CommandBuffers[b], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

    VkBuffer vertexBuffers[] = { m_VertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(m_CommandBuffers[b], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(m_CommandBuffers[b], m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(m_CommandBuffers[b], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[b], 0, nullptr);

    vkCmdDrawIndexed(m_CommandBuffers[b], m_Indices.size(), 1, 0, 0, 0);

    // ImGui start
    vkCmdBindPipeline(m_CommandBuffers[b], VK_PIPELINE_BIND_POINT_GRAPHICS, m_ImGuiPipeline);
    vkCmdBindDescriptorSets(m_CommandBuffers[b], VK_PIPELINE_BIND_POINT_GRAPHICS, m_ImGuiPipelineLayout, 0, 1, &m_ImGuiDescriptorSet, 0, nullptr);
    
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_SwapChainExtent.width;
    viewport.height = (float)m_SwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_CommandBuffers[b], 0, 1, &viewport);
    
    ImGuiIO& io = ImGui::GetIO();
    
    m_ImGuiConst.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    m_ImGuiConst.translate = glm::vec2(-1.0f);
    vkCmdPushConstants(m_CommandBuffers[b], m_ImGuiPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ImGuiConst), &m_ImGuiConst);
    
    // Render commands
    ImDrawData* imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;
    
    if (imDrawData && imDrawData->CmdListsCount > 0) {
    
      VkDeviceSize offsets[1] = { 0 };
      vkCmdBindVertexBuffers(m_CommandBuffers[b], 0, 1, &m_ImGuiVertexBuffer, offsets);
      vkCmdBindIndexBuffer(m_CommandBuffers[b], m_ImGuiIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
    
      for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
      {
        const ImDrawList* cmd_list = imDrawData->CmdLists[i];
        for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
        {
          const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
          VkRect2D scissorRect;
          scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
          scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
          scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
          scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
    
          vkCmdSetScissor(m_CommandBuffers[b], 0, 1, &scissorRect);
          vkCmdDrawIndexed(m_CommandBuffers[b], pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
          indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
      }
    }

    vkCmdEndRenderPass(m_CommandBuffers[b]);

    if (vkEndCommandBuffer(m_CommandBuffers[b]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void VulkanSubsystem::DrawFrame()
{ 
  ImGui::Render();

  vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, (std::numeric_limits<uint64_t>::max)());
  vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

  FreeCommandBuffers();
  CreateCommandBuffers();

  uint32_t imageIndex;
  vkAcquireNextImageKHR(m_Device, m_SwapChain, (std::numeric_limits<uint64_t>::max)(), m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

  UpdateUniformBuffer(imageIndex);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

  VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = { m_SwapChain };
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;
  vkQueuePresentKHR(m_PresentQueue, &presentInfo);

  m_CurrentFrame = (m_CurrentFrame + 1) % MaxFramesInFlight;
  m_TotalFrame++;
}

void VulkanSubsystem::CreateSyncObjects()
{
  m_ImageAvailableSemaphores.resize(MaxFramesInFlight);
  m_RenderFinishedSemaphores.resize(MaxFramesInFlight);
  m_InFlightFences.resize(MaxFramesInFlight);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (size_t i = 0; i < MaxFramesInFlight; i++)
  {
    if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
      vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
      vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]))
    {
      throw std::runtime_error("failed to create semaphores!");
    }
  }
}

void VulkanSubsystem::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
}

void VulkanSubsystem::CreateVertexBuffer()
{
  VkDeviceSize bufferSize = sizeof(Vertex) * m_Vertices.size();
  
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, m_Vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(m_Device, stagingBufferMemory);

  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

  CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

  vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
  vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void VulkanSubsystem::CreateIndexBuffer()
{
  VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, m_Indices.data(), (size_t)bufferSize);
  vkUnmapMemory(m_Device, stagingBufferMemory);

  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

  CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

  vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
  vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

uint32_t VulkanSubsystem::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
  {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanSubsystem::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
  
  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(commandBuffer);
}

void VulkanSubsystem::CreateDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding uboLayoutBinding = {};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = bindings.size();
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void VulkanSubsystem::CreateUniformBuffers()
{
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  m_UniformBuffers.resize(m_SwapChainImages.size());
  m_UniformBuffersMemory.resize(m_SwapChainImages.size());

  for (size_t i = 0; i < m_SwapChainImages.size(); i++)
  {
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);
  }
}

void VulkanSubsystem::UpdateUniformBuffer(uint32_t currentImage)
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
  UniformBufferObject ubo = {};
  const GameState& gameState = gApplicationInstanceManager.GetInstance(m_Window).GetGameState();
  auto model = glm::rotate(glm::mat4(1.0f), glm::radians(gameState.GetRotation(0)), glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(gameState.GetRotation(1)), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, glm::radians(gameState.GetRotation(2)), glm::vec3(0.0f, 0.0f, 1.0f));
  
  ubo.model = model;
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChainExtent.width / (float)m_SwapChainExtent.height, 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;

  void* data;
  vkMapMemory(m_Device, m_UniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(m_Device, m_UniformBuffersMemory[currentImage]);
}

void VulkanSubsystem::CreateDescriptorPool()
{
  std::array<VkDescriptorPoolSize, 3> poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[2].descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = poolSizes.size();
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(m_SwapChainImages.size()) + 1;

  if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) 
  {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void VulkanSubsystem::CreateDescriptorSets()
{
  std::vector<VkDescriptorSetLayout> layouts(m_SwapChainImages.size(), m_DescriptorSetLayout);
  layouts.push_back(m_ImGuiDescriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_DescriptorPool;
  allocInfo.descriptorSetCount = layouts.size();
  allocInfo.pSetLayouts = layouts.data();

  m_DescriptorSets.resize(m_SwapChainImages.size() + 1);
  VK_CHECK(vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data()));
  m_ImGuiDescriptorSet = m_DescriptorSets[m_SwapChainImages.size()];

  for (size_t i = 0; i < m_SwapChainImages.size(); i++)
  {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_UniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_TextureImageView;
    imageInfo.sampler = m_TextureSampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_DescriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr; // Optional
    descriptorWrites[0].pTexelBufferView = nullptr; // Optional

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_DescriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_Device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
  }

  // ImGui
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = m_ImGuiFontView;
  imageInfo.sampler = m_ImGuiFontSampler;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = m_DescriptorSets[m_SwapChainImages.size()];
  descriptorWrite.dstBinding = 1;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
}

void VulkanSubsystem::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(m_Device, image, imageMemory, 0);
}

void VulkanSubsystem::CreateTextureImage()
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(*gAssetManager.GetAsset("cube_DefaultMaterial_BaseColor")->m_Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels)
  {
    throw std::runtime_error("failed to load texture image!");
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(m_Device, stagingBufferMemory);

  stbi_image_free(pixels);

  CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);
  TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
  TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
  vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

VkCommandBuffer VulkanSubsystem::BeginSingleTimeCommands()
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_CommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void VulkanSubsystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_GraphicsQueue);

  vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void VulkanSubsystem::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (HasStencilComponent(format))
    {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }
  else
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else
  {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  EndSingleTimeCommands(commandBuffer);
}

void VulkanSubsystem::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = {
    width,
    height,
    1
  };

  vkCmdCopyBufferToImage(
    commandBuffer,
    buffer,
    image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
  );

  EndSingleTimeCommands(commandBuffer);
}

void VulkanSubsystem::CreateTextureImageView()
{
  m_TextureImageView = CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView VulkanSubsystem::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create texture image view!");
  }

  return imageView;
}

void VulkanSubsystem::CreateTextureSampler()
{
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 16;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create texture sampler!");
  }
}

void VulkanSubsystem::CreateDepthResources()
{
  VkFormat depthFormat = FindDepthFormat();
  CreateImage(m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
  m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
  TransitionImageLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat VulkanSubsystem::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
  for (VkFormat format : candidates)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
    {
      return format;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
    {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanSubsystem::FindDepthFormat()
{
  return FindSupportedFormat(
  { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

bool VulkanSubsystem::HasStencilComponent(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanSubsystem::LoadModelFBX()
{
  FbxManager* fbxSdkManager = FbxManager::Create();
  FbxIOSettings *ios = FbxIOSettings::Create(fbxSdkManager, IOSROOT);
  fbxSdkManager->SetIOSettings(ios);

  FbxImporter* fbxImporter = FbxImporter::Create(fbxSdkManager, "");

  const char* filename = *gAssetManager.GetAsset("cube")->m_Path;
  // Use the first argument as the filename for the importer.
  if (!fbxImporter->Initialize(filename, -1, fbxSdkManager->GetIOSettings()))
  {
    printf("Call to FbxImporter::Initialize() failed.\n");
    printf("Error returned: %s\n\n", fbxImporter->GetStatus().GetErrorString());
    exit(-1);
  }

  FbxScene* fbxScene = FbxScene::Create(fbxSdkManager, "myScene");
  fbxImporter->Import(fbxScene);
  fbxImporter->Destroy();

  //TODO: recursion
  if (FbxNode* rootNode = fbxScene->GetRootNode())
  {
    for (int nodeIdx = 0; nodeIdx < rootNode->GetChildCount(); ++nodeIdx)
    {
      FbxNode* childNode = rootNode->GetChild(nodeIdx);
      if (FbxMesh* mesh = childNode->GetMesh())
      {
        FbxGeometryElementUV* leUV = mesh->GetElementUV(0);
        for (int polyIdx = 0; polyIdx < mesh->GetPolygonCount(); ++polyIdx)
        {
          for (int subIdx = 0; subIdx < mesh->GetPolygonSize(polyIdx); ++subIdx)
          //for (int vertexIdx = 0; vertexIdx < mesh->GetControlPointsCount(); ++vertexIdx)
          {
            FbxVector4 position = mesh->GetControlPointAt(mesh->GetPolygonVertex(polyIdx, subIdx));
            
            Vertex vertex = {};

            vertex.m_Position =
            {
              float(position[0]),
              float(position[1]),
              float(position[2])
            };

            vertex.m_Color = { 1,1,1 };
            int lTextureUVIndex = mesh->GetTextureUVIndex(polyIdx, subIdx);
            FbxVector2 uv = leUV->GetDirectArray().GetAt(lTextureUVIndex);
            vertex.m_TexCoord = { uv[0], 1 - uv[1] };
            m_Vertices.push_back(vertex);
          }
        }

        //int* vertices = mesh->GetPolygonVertices();
        for (int indexIdx = 0; indexIdx < mesh->GetPolygonVertexCount(); indexIdx += 4)
        {
          m_Indices.push_back(indexIdx + 0);
          m_Indices.push_back(indexIdx + 1);
          m_Indices.push_back(indexIdx + 2);
          m_Indices.push_back(indexIdx + 2);
          m_Indices.push_back(indexIdx + 3);
          m_Indices.push_back(indexIdx + 0);
        }
      }
    }
  }
}

void VulkanSubsystem::LoadModelTinyObj()
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, *gAssetManager.GetAsset("chalet")->m_Path))
  {
    throw std::runtime_error(warn + err);
  }

  for (const auto& shape : shapes)
  {
    for (const auto& index : shape.mesh.indices)
    {
      Vertex vertex = {};

      vertex.m_Position = 
      {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      };

      vertex.m_TexCoord = {
        attrib.texcoords[2 * index.texcoord_index + 0],
        1.f - attrib.texcoords[2 * index.texcoord_index + 1]
      };

      vertex.m_Color = { 1.0f, 1.0f, 1.0f };

      m_Vertices.push_back(vertex);
      m_Indices.push_back(m_Indices.size());
    }
  }
}