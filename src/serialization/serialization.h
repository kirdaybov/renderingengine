#pragma once
#include <string>
#include <fstream>

struct SerializationContext;

class ISerializable
{
public:
  virtual ~ISerializable() {}
private:
  virtual void Serialize(SerializationContext& ctx) = 0;

  friend struct Serialization;
};

struct Serialization
{
  static void Serialize(const std::string& filename, ISerializable* object, bool read);
};

struct SerializationContext
{
  SerializationContext(std::ifstream* fstream)
    : m_Read(true)
    , m_InputStream(fstream)
  {}

  SerializationContext(std::ofstream* fstream)
    : m_Read(false)
    , m_OutputStream(fstream)
  {}

  std::ifstream* m_InputStream;
  std::ofstream* m_OutputStream;
  bool m_Read;

  void Serialize(int& data)
  {
    if (m_Read)
    {
      m_InputStream->read((char*)&data, sizeof(int));
    }
    else
    {
      m_OutputStream->write((char*)&data, sizeof(int));
    }
  }

  void Serialize(int& size, char*& data)
  {
    Serialize(size);
    if (m_Read)
    {
      data = new char[size]; //TODO: memory allocation
      m_InputStream->read(data, size);
    }
    else
    {
      m_OutputStream->write(data, size);
    }
  }
};

