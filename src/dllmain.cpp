// clang-format off
//
//    Copyright (C) 2020 Berkay Yigit <berkaytgy@gmail.com>
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License as published
//    by the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// clang-format on

#include "pch.h"
#include "Helpers/D3D9/D3D9Extender.hpp"
#include "Helpers/D3D11/D3D11Extender.hpp"

namespace MirrorHookInternals {
  enum class Framework { None, D3D9, D3D11 };

  std::mutex mMutex;
  bool       mLetInitRun = true;

  bool __stdcall InitWithWindowTitle(Framework framework, const wchar_t* const szWindowTitle) {
#pragma ExportedFunction
    std::scoped_lock<std::mutex> _l(mMutex);

    mLetInitRun = false;
    if (szWindowTitle) {
      switch (framework) {
        case Framework::D3D9:
          if (D3D9Extender::Init(szWindowTitle)) return true;
          break;
        case Framework::D3D11:
          if (D3D11Extender::Init(szWindowTitle)) return true;
          break;
      }
    }
    return false;
  }
  bool __stdcall InitWithWindowHandle(Framework framework, HWND& hWindow) {
#pragma ExportedFunction
    std::scoped_lock<std::mutex> _l(mMutex);

    mLetInitRun = false;
    switch (framework) {
      case Framework::D3D9:
        if (D3D9Extender::Init(hWindow)) return true;
        break;
      case Framework::D3D11:
        if (D3D11Extender::Init(hWindow)) return true;
        break;
    }
    return false;
  }
  bool __stdcall InitWithDevicePointer(Framework framework, void** ppDevice) {
#pragma ExportedFunction
    std::scoped_lock<std::mutex> _l(mMutex);

    mLetInitRun = false;
    if (ppDevice && framework == Framework::D3D9 && D3D9Extender::Init(reinterpret_cast<LPDIRECT3DDEVICE9*>(ppDevice)))
      return true;
    return false;
  }
}  // namespace MirrorHookInternals

void Init() {
  thread_local HWND _hWnd = nullptr;

  while (MirrorHookInternals::mLetInitRun) {
    EnumWindows(
        [](HWND hWnd, LPARAM lParam) -> BOOL {
          DWORD pId;
          GetWindowThreadProcessId(hWnd, &pId);

          if (GetCurrentProcessId() == pId) {
            TCHAR szClassName[MAX_PATH];
            if (GetClassName(hWnd, szClassName, _countof(szClassName))) {
#ifdef UNICODE
              if (wcscmp(szClassName, L"ConsoleWindowClass") == 0) {
#else
              if (strcmp(szClassName, "ConsoleWindowClass") == 0) {
#endif
                return TRUE;
              }

              *(HWND*)lParam = hWnd;
              return FALSE;
            }
          }
          return TRUE;
        },
        (LPARAM)&_hWnd);

    if (_hWnd) {
      if (GetModuleHandle(TEXT("d3d9.dll")))
        MirrorHookInternals::InitWithWindowHandle(MirrorHookInternals::Framework::D3D9, _hWnd);
      if (GetModuleHandle(TEXT("d3d11.dll")))
        MirrorHookInternals::InitWithWindowHandle(MirrorHookInternals::Framework::D3D11, _hWnd);

      break;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
  if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    std::thread(Init).detach();
  }
  return TRUE;
}
