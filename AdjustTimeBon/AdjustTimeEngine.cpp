#include "pch.h"
#include "AdjustTimeEngine.h"

namespace AdjustTimeBon {

AdjustTimeEngine::AdjustTimeEngine(int offset) noexcept
  :m_Offset(offset) {}


bool AdjustTimeEngine::WaitForTOTUpdated(const std::chrono::milliseconds& timeout) {

  LibISDB::BlockLock lock(m_EndLock);
  return m_EndCondition.WaitFor(m_EndLock, timeout, [this]() -> bool { return m_IsTOTUpdated.load(std::memory_order_acquire); });
}


void AdjustTimeEngine::OnTOTUpdated(LibISDB::AnalyzerFilter* pAnalyzer) {

  LibISDB::DateTime totTime;
  if (!pAnalyzer->GetTOTTime(&totTime)) {
    return;
  }

  SYSTEMTIME st = totTime.ToSYSTEMTIME();
  OffsetSystemTime(&st, -1 * m_Offset);
  if (::SetLocalTime(&st)) {
    tcout << _T("TOTで時刻合わせを行いました。(")
      << st.wYear
      << _T('/')
      << std::setw(2) << std::setfill(_T('0')) << st.wMonth
      << _T('/')
      << std::setw(2) << std::setfill(_T('0')) << st.wDay
      << _T(' ')
      << std::setw(2) << std::setfill(_T('0')) << st.wHour
      << _T(':')
      << std::setw(2) << std::setfill(_T('0')) << st.wMinute
      << _T(':')
      << std::setw(2) << std::setfill(_T('0')) << st.wSecond
      << _T(")") << std::endl;
    m_IsTOTUpdated.store(true, std::memory_order_release);
    m_EndCondition.NotifyOne();
  }
}


bool AdjustTimeEngine::OffsetSystemTime(SYSTEMTIME* pTime, int Offset) noexcept {

  FILETIME ft;
  if (!::SystemTimeToFileTime(pTime, &ft)) {
    return false;
  }

  ULARGE_INTEGER u;
  u.LowPart = ft.dwLowDateTime;
  u.HighPart = ft.dwHighDateTime;
  u.QuadPart += Offset * 10000LLU;
  ft.dwLowDateTime = u.LowPart;
  ft.dwHighDateTime = u.HighPart;

  return ::FileTimeToSystemTime(&ft, pTime) != FALSE;
}

}