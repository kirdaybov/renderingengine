#pragma once
#include <vulkan/vulkan.h>

class Buffer
{
public:
  Buffer();
  void Cleanup();
  void CopyDataToBufferMemory(VkDevice device, VkDeviceSize size, void* data);
  void MapMemory(void*& mappedData);
  void UnmapMemory();
  const VkBuffer& GetBuffer() { return m_Buffer; }
  const VkDeviceMemory& GetBufferMemory() { return m_BufferMemory; }  
private:
  VkBuffer m_Buffer;
  VkDeviceMemory m_BufferMemory;
  bool m_Mapped;
  friend class Renderer;
};