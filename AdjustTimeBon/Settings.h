#pragma once

#include <windows.h>

namespace AdjustTimeBon {

class Settings {

public:
  Settings() noexcept;

  Settings(Settings const&) = delete;
  void operator = (Settings const&) = delete;

  TCHAR m_DriverFile[MAX_PATH] = {};
  UINT m_Space = 0;
  UINT m_Channel = 0;
  UINT m_Offset = 0;
  UINT m_Timeout = 0;
};

}
