#pragma once
#include "uid/uid.h"
#include <unordered_map>

class InputSwitch
{
  int m_Key = 0;
  bool m_State = false;
public:
  InputSwitch(int key) : m_Key(key) {}
  void Update(int key, bool newState);
  inline bool IsOn() const { return m_State; }
};

class InputState
{
public:
  void KeyCallback(int key, int scancode, int action, int mods);
  void AddSwitch(UID uid, int key);
  bool IsSwitchOn(UID uid) const;
private:
  std::unordered_map<UID, InputSwitch> m_Switches;
};