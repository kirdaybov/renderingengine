#include "apptimer.h"
#include "imgui.h"

void AppTimer::Start()
{
  m_Last = std::chrono::high_resolution_clock::now();
}

void AppTimer::Update()
{
  TimeType newLast = std::chrono::high_resolution_clock::now();
  m_ElapsedSeconds = std::chrono::duration_cast<std::chrono::microseconds>(newLast - m_Last).count()/1000000.f;
  m_Last = newLast;

  float sum = 0.f;
  m_TimeRecords[m_LastRecordIdx] = m_ElapsedSeconds;
  m_LastRecordIdx = m_LastRecordIdx == KeptRecordsNum - 1 ? 0 : m_LastRecordIdx + 1;
  for (int i = 0; i < KeptRecordsNum; i++)
  {
    sum += m_TimeRecords[i];
  }

  if (ImGui::Begin("Frame stat"))
  {
    ImGui::Text("Frame time: %f ms", static_cast<float>(sum/KeptRecordsNum));
    ImGui::Text("FPS: %d", static_cast<int>(KeptRecordsNum/sum));
    ImGui::End();
  }
}