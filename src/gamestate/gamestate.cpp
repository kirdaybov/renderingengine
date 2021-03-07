#include "gamestate.h"
#include "apptimer/apptimer.h"
#include "input/inputstate.h"
#include "GLFW/glfw3.h"
#include "imgui.h"

void GameState::Update()
{
  if (ImGui::Begin("Scene"))
  {
    ImGui::SliderFloat3("Rotation", m_Rotation, 0, 180);
    ImGui::SliderFloat3("Sun Light", m_SunLightDirection, -1, 1);
    ImGui::End();
  }
  if (m_InputState->IsSwitchOn("-x"))
    AddRotation(0, -1.f);
  if (m_InputState->IsSwitchOn("+x"))
    AddRotation(0, 1.f);
  if (m_InputState->IsSwitchOn("-y"))
    AddRotation(1, -1.f);
  if (m_InputState->IsSwitchOn("+y"))
    AddRotation(1, 1.f);
  if (m_InputState->IsSwitchOn("-z"))
    AddRotation(2, -1.f);
  if (m_InputState->IsSwitchOn("+z"))
    AddRotation(2, 1.f);
}

void GameState::AddRotation(char dim, float amount)
{
  const float speed = 90.f;
  m_Rotation[dim] += speed*m_Timer->GetFrameSeconds()*amount;
}