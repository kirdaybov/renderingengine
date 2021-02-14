#pragma once
#include  "serialization/serialization.h"

class ShaderBinary : public ISerializable
{
  int m_Date;
  int m_BinarySize;
  char* m_Binary = nullptr;
public:
  ~ShaderBinary()
  {
    if (m_Binary)
    {
      delete[] m_Binary;
    }
  }
  void Serialize(SerializationContext& ctx) override
  {
    ctx.Serialize(m_Date);
    ctx.Serialize(m_BinarySize, m_Binary);
  }
  bool IsOld(int m_LastWriteTime) const { return m_Date < m_LastWriteTime; }
  void Compile(const std::string& name);
  char* GetData() { return m_Binary; }
  int GetSize() { return m_BinarySize; }
};

