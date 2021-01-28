#pragma once
#include <chrono>

// Use this class to acquire elapsed time between frames
class AppTimer
{
public:
  inline float GetFrameSeconds() const { return m_ElapsedSeconds; }
  void Start();
  void Update();

private:
  std::chrono::time_point<std::chrono::steady_clock> m_Last;
  float m_ElapsedSeconds = 0.f;
};