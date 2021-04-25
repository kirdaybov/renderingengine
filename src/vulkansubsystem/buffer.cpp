#include "buffer.h"
#include "app/application.h"

Buffer::Buffer()
  : m_Buffer(VK_NULL_HANDLE)
  , m_BufferMemory(VK_NULL_HANDLE)
  , m_Mapped(false)
{
}

void Buffer::Cleanup()
{
  if (m_Mapped && m_BufferMemory)
  {
    UnmapMemory();
  }
  if (m_Buffer)
  {
    vkDestroyBuffer(gRenderer.GetDevice(), m_Buffer, nullptr);
    m_Buffer = VK_NULL_HANDLE;
  }
  if (m_BufferMemory)
  {
    vkFreeMemory(gRenderer.GetDevice(), m_BufferMemory, nullptr);
    m_BufferMemory = VK_NULL_HANDLE;
  }
}

void Buffer::CopyDataToBufferMemory(VkDevice device, VkDeviceSize size, void* data)
{
  void* mappedData;
  MapMemory(mappedData);
  memcpy(mappedData, data, (size_t)size);
  UnmapMemory();
}

void Buffer::MapMemory(void*& mappedData)
{
  ASSERT(!m_Mapped);
  VK_CHECK(vkMapMemory(gRenderer.GetDevice(), m_BufferMemory, 0, VK_WHOLE_SIZE, 0, &mappedData));
  m_Mapped = true;
}

void Buffer::UnmapMemory()
{
  ASSERT(m_Mapped);
  vkUnmapMemory(gRenderer.GetDevice(), m_BufferMemory);
  m_Mapped = false;
}