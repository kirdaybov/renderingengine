#pragma once

#include <map>
#include <string>

class UIDManager;
  
class UID
{
  enum { InvalidUID = -1 };

public:
  UID(int id) : m_Id(id) {}
  UID(const char* key);

  inline bool operator ==(const UID &other) const { return m_Id == other.m_Id; }
  inline bool operator !=(const UID &other) const { return m_Id != other.m_Id; }
  inline bool operator <(const UID &other) const { return m_Id < other.m_Id; }
  inline operator int() const { return m_Id; }
  std::string ToString() const;
private:
  int m_Id = InvalidUID;
};

namespace std
{
  template <> struct hash<UID>
  {
    size_t operator()(const UID& uid) const
    {
      return (int)uid;
    }
  };
}

class UIDManager
{ 
public:    
  int GetUIDByName(const char* key);
  std::string GetNameByUID(int id);
  static UIDManager& GetInstance() { return ms_UIDManager; }
private:
  static UIDManager ms_UIDManager;
  static int ms_LastId;
  std::map<std::string, UID> m_UIDs;   
};