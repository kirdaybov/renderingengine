#include "buffer.h"
#include "app/application.h"
#include "imgui.h"

Buffer::Buffer(BufferPool& bufferPool)
  : m_Buffer(VK_NULL_HANDLE)
  , m_BufferMemory(VK_NULL_HANDLE)
  , m_Mapped(false)
  , m_BufferPool(bufferPool)
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
  MapMemory();
  memcpy(m_Mapped, data, (size_t)size);
  UnmapMemory();
}

void Buffer::MapMemory()
{
  ASSERT(!m_Mapped);
  VK_CHECK(vkMapMemory(gRenderer.GetDevice(), m_BufferMemory, 0, VK_WHOLE_SIZE, 0, &m_Mapped));
}

void Buffer::UnmapMemory()
{
  ASSERT(m_Mapped);
  vkUnmapMemory(gRenderer.GetDevice(), m_BufferMemory);
  m_Mapped = nullptr;
}

void Buffer::Release()
{
  ASSERT(!m_Released);
  m_BufferPool.ReleaseBuffer(this);
  m_Released = true;
}

BufferPool::BufferPool()
{
  for (int i = 0; i < MaxBuffers; i++)
  {
    m_Buffers.push_back(new Buffer(*this));
  }
}

BufferPool::~BufferPool()
{
}

Buffer* BufferPool::GetBuffer()
{
  Buffer* buffer = m_Buffers.front();
  buffer->m_Released = false;
  m_Buffers.pop_front();
  m_BuffersInUse++;
  return buffer;
}

void BufferPool::ReleaseBuffer(Buffer* buffer)
{
  m_ReleasedBuffers[m_Frame].push_back(buffer);
}

void BufferPool::Update()
{
  if (ImGui::Begin("Buffer Pool"))
  {
    ImGui::Text("Buffers in use %i/%i", m_BuffersInUse, MaxBuffers);
    ImGui::End();
  }
  int updateIdx = (m_Frame + 1) % 2;
  for (auto releasedBuffer : m_ReleasedBuffers[updateIdx])
  {
    releasedBuffer->Cleanup();
    m_Buffers.push_front(releasedBuffer);
    m_BuffersInUse--;
  }
  m_ReleasedBuffers[updateIdx].clear();
  m_Frame = updateIdx;
}
