#pragma once
#include <vulkan/vulkan.h>
#include <list>

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
  void Release();
private:
  bool m_Released = false;
  VkBuffer m_Buffer;
  VkDeviceMemory m_BufferMemory;
  bool m_Mapped;
  friend class Renderer;
  friend class BufferPool;
};

class BufferPool
{
public:
  BufferPool();
  ~BufferPool();
  enum { MaxBuffers = 10 };
  Buffer* GetBuffer();
  static BufferPool& GetBufferPool() { return ms_BufferPool; }
  void ReleaseBuffer(Buffer* buffer);
  void Update();
private:
  std::list<Buffer*> m_Buffers;
  std::list<Buffer*> m_ReleasedBuffers[2];
  int m_Frame = 0;
  static BufferPool ms_BufferPool;
};