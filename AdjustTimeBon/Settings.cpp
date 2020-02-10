#include "pch.h"
#include "Settings.h"

namespace AdjustTimeBon {

Settings::Settings() noexcept {

  TCHAR iniFilePath[MAX_PATH] = {};
  ::GetModuleFileName(nullptr, iniFilePath, MAX_PATH);
  size_t baseSize = ::_tcslen(iniFilePath) - 4;
  ::_tcsncpy_s(&iniFilePath[baseSize], MAX_PATH - baseSize, _T(".ini"), 4);

  ::GetPrivateProfileString(_T("SETTINGS"), _T("DRIVER_FILE"), _T("default.dll"), m_DriverFile, MAX_PATH, iniFilePath);
  m_Space = ::GetPrivateProfileInt(_T("SETTINGS"), _T("SPACE"), 0, iniFilePath);
  m_Channel = ::GetPrivateProfileInt(_T("SETTINGS"), _T("CHANNEL"), 0, iniFilePath);
  m_Offset = ::GetPrivateProfileInt(_T("SETTINGS"), _T("OFFSET"), 0, iniFilePath);
  m_Timeout = ::GetPrivateProfileInt(_T("SETTINGS"), _T("TIMEOUT"), 0, iniFilePath);
}

}
