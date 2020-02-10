#pragma once

#include <atomic>
#include "LibISDB/LibISDB.hpp"
#include "LibISDB/Engine/TSEngine.hpp"
#include "LibISDB/Utilities/ConditionVariable.hpp"

namespace AdjustTimeBon {

class AdjustTimeEngine
  : public LibISDB::TSEngine {

public:
  AdjustTimeEngine(int offset) noexcept;
  bool WaitForTOTUpdated(const std::chrono::milliseconds& timeout);

  AdjustTimeEngine(AdjustTimeEngine const&) = delete;
  void operator = (AdjustTimeEngine const&) = delete;

private:
  void OnTOTUpdated(LibISDB::AnalyzerFilter* pAnalyzer) override;

  static bool OffsetSystemTime(SYSTEMTIME* pTime, int Offset) noexcept;

  int m_Offset;
  LibISDB::ConditionVariable m_EndCondition;
  LibISDB::MutexLock m_EndLock;
  std::atomic<bool> m_IsTOTUpdated{ false };
};

}
