#include "serialization.h"
#include "logger/logger.h"

void Serialization::Serialize(const std::string& filename, ISerializable* object, bool read)
{
  if (read)
  {
    std::ifstream f;
    f.open(filename, std::ifstream::in | std::ifstream::binary);
    if (!f.is_open())
    {
      DEBUG_BREAK("Couldn't open %s", filename.data());
    }
    else
    {
      object->Serialize(SerializationContext(&f));
      f.close();
    }
  }
  else
  {
    std::ofstream f;
    f.open(filename, std::ofstream::out | std::ofstream::binary);
    object->Serialize(SerializationContext(&f));
    f.close();
  }
}
