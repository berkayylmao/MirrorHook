// clang-format off
// 
//    Miscellaneous Files (MirrorHook)
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
  #include <libloaderapi.h> // GetModuleHandle
  #include <WinDef.h>       // HWND
  #pragma warning(pop)

struct IDirect3DDevice9;
struct IDXGISwapChain;

namespace MirrorHook {
  enum class Framework { None, D3D9, D3D11 };

  inline bool Init(const Framework framework, const wchar_t* const szWindowTitle) {
    return reinterpret_cast<bool(__stdcall*)(const Framework, const wchar_t* const)>(
      GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::InitWithWindowTitle"))(framework, szWindowTitle);
  }
  inline bool Init(const Framework framework, const HWND& hWnd) {
    return reinterpret_cast<bool(__stdcall*)(const Framework, const HWND&)>(
      GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::InitWithWindowHandle"))(framework, hWnd);
  }
  inline bool Init(const Framework framework, IDirect3DDevice9** ppDevice) {
    return reinterpret_cast<bool(__stdcall*)(const Framework, IDirect3DDevice9**)>(
      GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::InitWithDevicePointer"))(framework, ppDevice);
  }

  namespace D3D9 {
    enum class Type { BeginScene, EndScene, BeforeReset, AfterReset };

    inline bool AddExtension(const Type type, void(__stdcall* pExtension)(IDirect3DDevice9* pDevice)) {
      return reinterpret_cast<bool(__stdcall*)(const Type, void(__stdcall*)(IDirect3DDevice9*))>(
        GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D9Extender::details::AddExtension"))(type, pExtension);
    }
    inline bool AddExtension(const Type type, void(__stdcall* pExtension)(IDirect3DDevice9* pDevice, void* pPresentationParameters)) {
      return reinterpret_cast<bool(__stdcall*)(const Type, void(__stdcall*)(IDirect3DDevice9*, void*))>(
        GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D9Extender::details::AddExtension"))(type, pExtension);
    }
  } // namespace D3D9
  namespace D3D11 {
    enum class Type { Present };

    inline bool AddExtension(const Type type, void(__stdcall* pExtension)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)) {
      return reinterpret_cast<bool(__stdcall*)(const Type, void(__stdcall*)(IDXGISwapChain*, UINT, UINT))>(
        GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D11Extender::details::AddExtension"))(type, pExtension);
    }
  } // namespace D3D11
  namespace WndProc {
    constexpr LRESULT g_constIgnoreThisReturn = -1;

    inline void AddExtension(LRESULT(__stdcall* pExtension)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)) {
      reinterpret_cast<void(__stdcall*)(LRESULT(__stdcall*)(HWND, UINT, WPARAM, LPARAM))>(
        GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::WndProcExtender::details::AddExtension"))(pExtension);
    }
    inline HWND GetWindowHandle() {
      return reinterpret_cast<HWND(__stdcall*)()>(
        GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::WndProcExtender::details::GetWindowHandle"))();
    }
  } // namespace WndProc
} // namespace MirrorHook
#endif
