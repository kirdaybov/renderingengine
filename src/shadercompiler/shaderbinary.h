#pragma once
#include  "serialization/serialization.h"

class ShaderBinary : public ISerializable
{
  int m_Date;
  int m_BinarySize;
  char* m_Binary = nullptr;

  // false if the last updated code won't compile
  bool m_IsCompiled = false;
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
    if (ctx.m_Read && m_BinarySize > 0)
    {
      m_IsCompiled = true;
    }
  }
  bool IsOld(int m_LastWriteTime) const { return m_Date < m_LastWriteTime; }
  bool IsCompiled() const { return m_IsCompiled; }
  bool Compile(const std::string& name);
  char* GetData() { return m_Binary; }
  int GetSize() { return m_BinarySize; }
};

