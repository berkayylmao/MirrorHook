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

  bool __stdcall InitWithWindowTitle(Framework framework, const wchar_t* const szWindowTitle) {
#pragma ExportedFunction
    std::scoped_lock<std::mutex> _l(mMutex);

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

    if (ppDevice && framework == Framework::D3D9 && D3D9Extender::Init(reinterpret_cast<LPDIRECT3DDEVICE9*>(ppDevice)))
      return true;
    return false;
  }
}  // namespace MirrorHookInternals

void Init() {
  EnumWindows(
      [](HWND hWnd, LPARAM) -> BOOL {
        DWORD _pId;
        GetWindowThreadProcessId(hWnd, &_pId);

        if (GetCurrentProcessId() == _pId) {
          TCHAR _szClassName[MAX_PATH];
          if (GetClassName(hWnd, _szClassName, _countof(_szClassName))) {
#if defined(UNICODE)
            if (std::wcscmp(_szClassName, L"ConsoleWindowClass") == 0) {
#else
            if (std::strcmp(_szClassName, "ConsoleWindowClass") == 0) {
#endif
              if (GetModuleHandle(TEXT("d3d9.dll")))
                MirrorHookInternals::InitWithWindowHandle(MirrorHookInternals::Framework::D3D9, hWnd);
              if (GetModuleHandle(TEXT("d3d11.dll")))
                MirrorHookInternals::InitWithWindowHandle(MirrorHookInternals::Framework::D3D11, hWnd);
            }
          }
        }
        return TRUE;
      },
      NULL);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
  if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    std::thread(Init).detach();
  }
  return TRUE;
}
