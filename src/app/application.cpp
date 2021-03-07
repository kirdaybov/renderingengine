#include "application.h"
#include "logger/logger.h"
#include "imgui.h"

void CALLBACK MidiInProc(
  HMIDIIN   hMidiIn,
  UINT      wMsg,
  DWORD_PTR dwInstance,
  DWORD_PTR dwParam1,
  DWORD_PTR dwParam2
)
{
  switch (wMsg)
  {
  case MIM_DATA: LOGF("wMsg=MIM_DATA, dwInstance=%08x, dwParam1=%08x, dwParam2=%08x\n", 
    static_cast<unsigned int>(dwInstance), static_cast<unsigned int>(dwParam1), static_cast<unsigned int>(dwParam2)); break;
  };
}

void GlobalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  gApplicationInstanceManager.GetSingletone().GetInstance(window).KeyCallback(key, scancode, action, mods);
}

void GlobalMouseKeyCallback(GLFWwindow* window, int key, int action, int mods)
{
  gApplicationInstanceManager.GetSingletone().GetInstance(window).MouseKeyCallback(key, action, mods);
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
    int numMidiDevs = midiInGetNumDevs();
    if (numMidiDevs > 0)
    {
      LOGF("Midi devices found: %i", numMidiDevs);
      HMIDIIN hMidiInput;
      midiInOpen(&hMidiInput, 0, (DWORD_PTR)MidiInProc, 0, CALLBACK_FUNCTION);
      midiInStart(hMidiInput);
    }
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
      ImGui::NewFrame();

      ImGuiIO& io = ImGui::GetIO();

      io.MousePos = ImVec2(m_InputState.GetMouseX(), m_InputState.GetMouseY());
      io.MouseDown[0] = m_InputState.IsSwitchOn("LMB");
      io.MouseDown[1] = m_InputState.IsSwitchOn("RMB");

      m_AppTimer.Update();
      m_GameState.Update();
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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_Window = glfwCreateWindow(1920, 1080, "kirdaybov", nullptr, nullptr);
  if (!m_Window)
  {
    LOGF("Couldn't create window");
    return false;
  }

  //glfwMakeContextCurrent(m_Window); //TODO: what is this?
  return true;
}

ApplicationInstanceManager ApplicationInstanceManager::m_Singletone;