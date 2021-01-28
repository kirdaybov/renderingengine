#pragma once

class AppTimer;
class InputState;

class GameState
{
public:
  void Init(AppTimer* timer, InputState* inputState) { m_Timer = timer; m_InputState = inputState; }
  void AddRotation(char dim, float amount);
  inline float GetRotation(char dim) const { return m_Rotation[dim]; }
  void Update();
private:
  AppTimer* m_Timer = nullptr;
  InputState* m_InputState = nullptr;
  float m_Rotation[3] = { 0.f, 0.f, 0.f };
};