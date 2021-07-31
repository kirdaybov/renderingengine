#pragma once
#include <chrono>
#include <array>

class AppTimer
{
public:
  float GetFrameSeconds() const { return m_ElapsedSeconds; }

  void Start();
  void Update();

private:
  typedef std::chrono::time_point<std::chrono::steady_clock> TimeType;
  TimeType m_Last;
  float m_ElapsedSeconds = 0.f;

  enum { KeptRecordsNum = 100 };
  std::array<float, KeptRecordsNum> m_TimeRecords;
  int m_LastRecordIdx = 0;
};
