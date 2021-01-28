#include "apptimer.h"

void AppTimer::Start()
{
  m_Last = std::chrono::high_resolution_clock::now();
}

void AppTimer::Update()
{
  auto newLast = std::chrono::high_resolution_clock::now();
  m_ElapsedSeconds = std::chrono::duration_cast<std::chrono::microseconds>(newLast - m_Last).count()/1000000.f;
  m_Last = newLast;
}