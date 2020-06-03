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

#pragma once
#ifndef __MIRRORHOOK_INCLUDED__
#define __MIRRORHOOK_INCLUDED__

#pragma warning(push, 0)
#include <libloaderapi.h>  // GetModuleHandle
#include <WinDef.h>        // HWND
#pragma warning(pop)

namespace MirrorHook {
  enum class Framework { None, D3D9, D3D11 };

  static bool Init(Framework framework, const wchar_t* const szWindowTitle) {
    return reinterpret_cast<bool(__stdcall*)(Framework, const wchar_t* const)>(GetProcAddress(
        GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::InitWithWindowTitle"))(framework, szWindowTitle);
  }
  static bool Init(Framework framework, HWND& hWnd) {
    return reinterpret_cast<bool(__stdcall*)(Framework, HWND&)>(GetProcAddress(
        GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::InitWithWindowHandle"))(framework, hWnd);
  }
  static bool Init(Framework framework, void** ppDevice) {
    return reinterpret_cast<bool(__stdcall*)(Framework, void**)>(GetProcAddress(
        GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::InitWithDevicePointer"))(framework, ppDevice);
  }

  namespace D3D9 {
    enum class D3D9Extension { BeginScene, EndScene, BeforeReset, AfterReset };

    static bool AddExtension(D3D9Extension type, void* const pExtension) {
      return reinterpret_cast<bool(__stdcall*)(D3D9Extension, void* const)>(
          GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D9Extender::AddExtension"))(
          type, pExtension);
    }
  }  // namespace D3D9
  namespace D3D11 {
    enum class D3D11Extension { Present };

    static bool AddExtension(D3D11Extension type, void* const pExtension) {
      return reinterpret_cast<bool(__stdcall*)(D3D11Extension, void* const)>(
          GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D11Extender::AddExtension"))(
          type, pExtension);
    }
  }  // namespace D3D11
  namespace WndProc {
    constexpr LRESULT g_constIgnoreThisReturn = -1;

    static bool AddExtension(void* const pExtension) {
      return reinterpret_cast<bool(__stdcall*)(void* const)>(GetProcAddress(
          GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::WndProcExtender::AddExtension"))(pExtension);
    }
    static HWND GetWindowHandle() {
      return reinterpret_cast<HWND(__stdcall*)()>(GetProcAddress(
          GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::WndProcExtender::GetWindowHandle"))();
    }
  }  // namespace WndProc
}  // namespace MirrorHook
#endif
