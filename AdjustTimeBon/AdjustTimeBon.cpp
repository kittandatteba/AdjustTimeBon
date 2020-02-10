// AdjustTimeBon.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include "Settings.h"
#include "AdjustTimeEngine.h"
#include "LibISDB/Filters/TSPacketParserFilter.hpp"
#include "LibISDB/Windows/Filters/BonDriverSourceFilter.hpp"

namespace AdjustTimeBon {

class scoped_guard {

  std::function<void()> f;

public:

  explicit scoped_guard(std::function<void()> f)
    : f(f) {}

  scoped_guard(scoped_guard const&) = delete;
  void operator = (scoped_guard const&) = delete;


  ~scoped_guard() {
    f();
  }
};


void ChangePrivileges() noexcept(false) {

  HANDLE hToken;
  if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    tcerr << _T("プロセストークンの取得に失敗しました。") << std::endl;
    throw std::runtime_error(nullptr);
  }
  scoped_guard guard([&] { ::CloseHandle(hToken); });

  LUID luid;
  if (!::LookupPrivilegeValue(nullptr, SE_SYSTEMTIME_NAME, &luid)) {
    tcerr << _T("SE_SYSTEMTIME_NAME特権に対応するLUIDの取得に失敗しました。") << std::endl;
    throw std::runtime_error(nullptr);
  }

  TOKEN_PRIVILEGES tp;
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  if (!::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr) || ::GetLastError() != ERROR_SUCCESS) {
    tcerr << _T("SE_SYSTEMTIME_NAME特権の有効化に失敗しました。") << std::endl;
    throw std::runtime_error(nullptr);
  }
}


void AdjustTime() noexcept(false) {

  AdjustTimeBon::Settings settings;

  auto pSource = new LibISDB::BonDriverSourceFilter;
  pSource->SetPurgeStreamOnChannelChange(true);

  auto pParser = new LibISDB::TSPacketParserFilter;

  auto pAnalyzer = new LibISDB::AnalyzerFilter;

  AdjustTimeBon::AdjustTimeEngine engine(settings.m_Offset);
  engine.BuildEngine({ pSource, pParser, pAnalyzer });
  engine.SetStartStreamingOnSourceOpen(true);

  if (!engine.OpenSource(settings.m_DriverFile)) {
    tcerr << _T("BonDriverのオープンに失敗しました。") << engine.GetLastErrorText() << std::endl;
    throw std::runtime_error(nullptr);
  }
  scoped_guard guard([&] { engine.CloseSource(); });


  if (!pSource->SetChannel(settings.m_Space, settings.m_Channel)) {
    tcerr << _T("チャンネル設定に失敗しました。") << pSource->GetLastErrorText() << std::endl;
    // デフォルトのチャンネルで時刻合わせを試みるためthrowしない
  }

  std::chrono::milliseconds timeout(settings.m_Timeout);
  if (!engine.WaitForTOTUpdated(timeout)) {
    tcerr << _T("タイムアウト時間までにTOTが受信できませんでした。") << std::endl;
    throw std::runtime_error(nullptr);
  }
}

}


int _tmain(int argc, _TCHAR* argv[]) {

#ifdef _DEBUG
  ::_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
  ::_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

  ::_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
  ::_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);

  ::_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  ::_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

  ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF |
    _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  ::setlocale(LC_ALL, "japanese");

  try {
    AdjustTimeBon::ChangePrivileges();
    AdjustTimeBon::AdjustTime();
  } catch (const std::exception&) {
    tcerr << _T("時刻合わせに失敗しました。") << std::endl;
    return 1;
  }

#ifdef _DEBUG
  _CrtDumpMemoryLeaks();
#endif

  return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
