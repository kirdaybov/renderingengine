#pragma once

#include <Windows.h>
#include <GLFW/glfw3.h>
#include "vulkansubsystem/vulkansubsystem.h"
#include "gamestate/gamestate.h"
#include "input/inputstate.h"
#include "apptimer/apptimer.h"
#include "shadercompiler/shadercompiler.h"

class ApplicationInstance
{
  bool m_Stop = false;
  Renderer m_Renderer;
  GLFWwindow* m_Window;
  GameState m_GameState;
  InputState m_InputState;
  AppTimer m_AppTimer;
  ShaderCompiler m_ShaderCompiler;
public:
  void Run();
  void ScheduleStop() { m_Stop = true; }
  void Cleanup();
  void KeyCallback(int key, int scancode, int action, int mods);
  void MouseKeyCallback(int key, int action, int mods);

  GameState& GetGameState() { return m_GameState; }
  const GameState& GetGameState() const { return m_GameState; }
  InputState& GetInputState() { return m_InputState; }
  const InputState& GetInputState() const { return m_InputState; }
  AppTimer& GetTimer() { return m_AppTimer; }
  const AppTimer& GetTimer() const{ return m_AppTimer; }
  ShaderCompiler& GetShaderCompiler() { return m_ShaderCompiler; }
  Renderer& GetRenderer() { return m_Renderer; }

  GLFWwindow* GetWindow() const { return m_Window; }
private:
  bool RegisterCreateAndShowWindow();
};

#define gApplicationInstanceManager ApplicationInstanceManager::GetSingletone()
#define gRenderer ApplicationInstanceManager::GetSingletone().GetInstance().GetRenderer()

class ApplicationInstanceManager
{
  static ApplicationInstanceManager m_Singletone;
  ApplicationInstance m_Instance;
public:
  inline static ApplicationInstanceManager& GetSingletone() { return m_Singletone; }
  ApplicationInstance& GetInstance(GLFWwindow* window) { return m_Instance; } //TODO: remove
  ApplicationInstance& GetInstance() { return m_Instance; }
  bool StartNewInstance()
  {
    m_Instance.Run();
    return true;
  }
};