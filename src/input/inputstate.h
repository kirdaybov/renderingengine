#pragma once
#include "uid/uid.h"
#include <unordered_map>

class ApplicationInstance;

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
  void Init(ApplicationInstance* instance);
  void Update();
  void KeyCallback(int key, int scancode, int action, int mods);
  void MouseKeyCallback(int key, int action, int mods);
  void AddSwitch(UID uid, int key);
  bool IsSwitchOn(UID uid) const;
  float GetMouseX() const { return m_MouseX; }
  float GetMouseY() const { return m_MouseY; }
private:
  std::unordered_map<UID, InputSwitch> m_Switches;
  ApplicationInstance* m_ApplicationInstance;
  float m_MouseX;
  float m_MouseY;
};