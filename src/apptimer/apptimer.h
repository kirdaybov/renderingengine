#pragma once
#include <chrono>

class AppTimer
{
public:
  float GetFrameSeconds() const { return m_ElapsedSeconds; }

  void Start();
  void Update();

private:
  std::chrono::time_point<std::chrono::steady_clock> m_Last;
  float m_ElapsedSeconds = 0.f;
};