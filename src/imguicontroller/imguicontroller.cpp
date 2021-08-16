#include "imguicontroller.h"
#include "app/application.h"
#include "imgui.h"

void ImGuiController::Init()
{
}

void ImGuiController::Update()
{
  ImGui::NewFrame();

  ImGuiIO& io = ImGui::GetIO();
  //InputState& inputState = gInputState;
  io.MousePos = ImVec2(gInputState.GetMouseX(), gInputState.GetMouseY());
  io.MouseDown[0] = gInputState.IsSwitchOn("LMB");
  io.MouseDown[1] = gInputState.IsSwitchOn("RMB");
}

void ImGuiController::Render()
{
  ImGui::Render();
}

void ImGuiController::Cleanup()
{
}
