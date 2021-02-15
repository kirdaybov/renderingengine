#include "inputstate.h"
#include "app/application.h"

void InputState::Init(ApplicationInstance* instance)
{
  m_ApplicationInstance = instance;
  AddSwitch("LMB", 0);
  AddSwitch("RMB", 1);
}

void InputState::Update()
{
  double mouseX, mouseY;
  glfwGetCursorPos(m_ApplicationInstance->GetWindow(), &mouseX, &mouseY);
  m_MouseX = (float)mouseX;
  m_MouseY = (float)mouseY;
}

void InputState::KeyCallback(int key, int scancode, int action, int mods)
{
  for (auto& sw : m_Switches)
  {
    sw.second.Update(key, action != GLFW_RELEASE);
  }
}

void InputState::MouseKeyCallback(int key, int action, int mods)
{
  for (auto& sw : m_Switches)
  {
    sw.second.Update(key, action != GLFW_RELEASE);
  }
}

void InputState::AddSwitch(UID uid, int key)
{
  // TODO: assert uid exists
  m_Switches.insert(std::make_pair(uid, InputSwitch(key)));
}

bool InputState::IsSwitchOn(UID uid) const
{
  auto it = m_Switches.find(uid);
  if (it != m_Switches.end())
    return it->second.IsOn();
  return false;
}

void InputSwitch::Update(int key, bool newState)
{
  if (m_Key == key)
    m_State = newState;
}