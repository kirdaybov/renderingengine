#pragma once

#include <GLFW/glfw3.h>
#include "vulkansubsystem/vulkansubsystem.h"
#include "gamestate/gamestate.h"
#include "input/inputstate.h"
#include "apptimer/apptimer.h"

class ApplicationInstance
{
  bool m_Stop = false;
  VulkanSubsystem m_VulkanSubsystem;
  GLFWwindow* m_Window;
  GameState m_GameState;
  InputState m_InputState;
  AppTimer m_AppTimer;
public:
  void Run();
  void ScheduleStop() { m_Stop = true; }
  void Cleanup();
  void KeyCallback(int key, int scancode, int action, int mods);
  void MouseKeyCallback(int key, int action, int mods);

  inline GameState& GetGameState() { return m_GameState; }
  inline const GameState& GetGameState() const { return m_GameState; }
  inline InputState& GetInputState() { return m_InputState; }
  inline const InputState& GetInputState() const { return m_InputState; }
  inline AppTimer& GetTimer() { return m_AppTimer; }
  inline const AppTimer& GetTimer() const{ return m_AppTimer; }

  GLFWwindow* GetWindow() const { return m_Window; }
private:
  bool RegisterCreateAndShowWindow();
};

#define gApplicationInstanceManager ApplicationInstanceManager::GetSingletone()

class ApplicationInstanceManager
{
  static ApplicationInstanceManager m_Singletone;
  ApplicationInstance m_Instance;
public:
  inline static ApplicationInstanceManager& GetSingletone() { return m_Singletone; }
  ApplicationInstance& GetInstance(GLFWwindow* window) { return m_Instance; }
  bool StartNewInstance()
  {
    m_Instance.Run();
    return true;
  }
};