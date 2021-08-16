#pragma once
#include <vulkan/vulkan.h>
#include <list>

class BufferPool;

class Buffer
{
public:
  Buffer(BufferPool& bufferPool);
  void Cleanup();
  void CopyDataToBufferMemory(VkDevice device, VkDeviceSize size, void* data);
  void MapMemory();
  void UnmapMemory();
  const VkBuffer& GetBuffer() { return m_Buffer; }
  const VkDeviceMemory& GetBufferMemory() { return m_BufferMemory; }
  void Release();
  void* GetMapped() { return m_Mapped; }
private:
  bool m_Released = false;
  VkBuffer m_Buffer;
  VkDeviceMemory m_BufferMemory;
  void* m_Mapped;
  BufferPool& m_BufferPool;
  friend class Renderer;
  friend class BufferPool;
};

class BufferPool
{
public:
  BufferPool();
  ~BufferPool();
  enum { MaxBuffers = 50 };
  Buffer* GetBuffer();
  void ReleaseBuffer(Buffer* buffer);
  void Update();
private:
  std::list<Buffer*> m_Buffers;
  std::list<Buffer*> m_ReleasedBuffers[2];
  int m_Frame = 0;
  int m_BuffersInUse = 0;
};