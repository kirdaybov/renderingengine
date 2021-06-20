#include "application.h"
#include "logger/logger.h"

ApplicationInstance ApplicationInstance::m_Instance;

void GlobalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  gApp.KeyCallback(key, scancode, action, mods);
}

void GlobalMouseKeyCallback(GLFWwindow* window, int key, int action, int mods)
{
  gApp.MouseKeyCallback(key, action, mods);
}

void ApplicationInstance::MouseKeyCallback(int key, int action, int mods)
{
  m_InputState.MouseKeyCallback(key, action, mods);
}

void ApplicationInstance::KeyCallback(int key, int scancode, int action, int mods)
{
  m_InputState.KeyCallback(key, scancode, action, mods);
}

void ApplicationInstance::Run()
{
  if (!glfwInit())
  {
    LOGF("Couldn't initialize GLFW");
    return;
  } 

  if (RegisterCreateAndShowWindow())
  {
    m_AppTimer.Start();
    glfwSetKeyCallback(m_Window, GlobalKeyCallback);
    glfwSetMouseButtonCallback(m_Window, GlobalMouseKeyCallback);

    m_InputState.Init(this);

    m_ShaderCompiler.Init();
    m_Renderer.Init(m_Window);
    m_GameState.Init(&m_AppTimer, &m_InputState);

    m_InputState.AddSwitch("-x", GLFW_KEY_A);
    m_InputState.AddSwitch("+x", GLFW_KEY_D);
    m_InputState.AddSwitch("-y", GLFW_KEY_W);
    m_InputState.AddSwitch("+y", GLFW_KEY_S);
    m_InputState.AddSwitch("-z", GLFW_KEY_Q);
    m_InputState.AddSwitch("+z", GLFW_KEY_E);

    while (!glfwWindowShouldClose(m_Window))
    {
      glfwPollEvents();
      m_InputState.Update();
      m_ImGuiController.Update();
      
      m_AppTimer.Update();
      m_GameState.Update();
      m_ShaderCompiler.Update();
      m_Renderer.Update();

      // Render ImGui before drawing so that renderer could render it properly
      m_ImGuiController.Render();
      m_Renderer.DrawFrame();
    }
    m_Renderer.DeviceWaitIdle();

    Cleanup();
  }
}

void ApplicationInstance::Cleanup()
{
  m_Renderer.Cleanup();
  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

bool ApplicationInstance::RegisterCreateAndShowWindow()
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_Window = glfwCreateWindow(1920, 1080, "kirdaybov", nullptr, nullptr);
  if (!m_Window)
  {
    LOGF("Couldn't create window");
    return false;
  }

  //glfwMakeContextCurrent(m_Window); //TODO: what is this?
  return true;
}