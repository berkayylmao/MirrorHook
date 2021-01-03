// clang-format off
// 
//    MirrorHook (MirrorHook)
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

  static inline std::mutex InternalMutex;

  auto __stdcall InitWithWindowTitle(const Framework framework, const wchar_t* const szWindowTitle) -> bool {
#pragma __EXPORTED_FUNCTION__
    if (!szWindowTitle) return false;
    std::scoped_lock<std::mutex> _l(InternalMutex);

    switch (framework) {
      case Framework::D3D9:
        return D3D9Extender::Init(szWindowTitle);
      case Framework::D3D11:
        return D3D11Extender::Init(szWindowTitle);
      case Framework::None:
        return true;
    }

    return false;
  }

  auto __stdcall InitWithWindowHandle(const Framework framework, const HWND& hWindow) -> bool {
#pragma __EXPORTED_FUNCTION__
    std::scoped_lock<std::mutex> _l(InternalMutex);

    switch (framework) {
      case Framework::D3D9:
        return D3D9Extender::Init(hWindow);
      case Framework::D3D11:
        return D3D11Extender::Init(hWindow);
      case Framework::None:
        return true;
    }

    return false;
  }

  auto _stdcall InitWithDevicePointer(const Framework framework, void** ppDevice) -> bool {
#pragma __EXPORTED_FUNCTION__
    std::scoped_lock<std::mutex> _l(InternalMutex);

    return ppDevice && framework == Framework::D3D9 && D3D9Extender::Init(reinterpret_cast<LPDIRECT3DDEVICE9*>(ppDevice));
  }
} // namespace MirrorHookInternals

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
            if (GetModuleHandle(TEXT("d3d9.dll"))) MirrorHookInternals::InitWithWindowHandle(MirrorHookInternals::Framework::D3D9, hWnd);
            if (GetModuleHandle(TEXT("d3d11.dll"))) MirrorHookInternals::InitWithWindowHandle(MirrorHookInternals::Framework::D3D11, hWnd);
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
