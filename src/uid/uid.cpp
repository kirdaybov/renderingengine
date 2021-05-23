#include "uid.h"

#include <cassert>
#include <iostream>

int UIDManager::ms_LastId = 0;

UIDManager UIDManager::ms_UIDManager = UIDManager();

UID::UID(const char* id_string)
{
  m_Id = UIDManager::GetInstance().GetUIDByName(id_string);
}

std::string UID::ToString() const 
{
  return UIDManager::GetInstance().GetNameByUID(m_Id);
}

int UIDManager::GetUIDByName(const char* key)
{
  auto it = m_UIDs.find(key);
  if (it != m_UIDs.end())
    return it->second;
  m_UIDs.insert(std::make_pair(std::string(key), UID(ms_LastId++)));
  return m_UIDs.find(key)->second;
}

std::string UIDManager::GetNameByUID(int id)
{
  for (auto& pair : m_UIDs)
  {
    if (pair.second == UID(id))
    {
      return pair.first;
    }
  }
  return std::string();
}

struct _test_UID
{
  _test_UID()
  {
    UID id1 = 2;
    UID id2 = 3;
    UID id3 = 2;

    assert(id1 != id2);
    assert(id1 == id3);

    UID id4 = "one";
    UID id5 = "four";
    UID id6 = "two";

    assert(id4 != id5);
    assert(id4 != id6);
    assert(id5 == "four");
  }
} _test_UID_;