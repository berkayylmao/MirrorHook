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
  #include <WinDef.h>       // HWND, HMODULE, TCHAR, FARPROC
  #pragma warning(pop)

struct IDirect3DDevice9;
struct IDXGISwapChain;

namespace MirrorHook {
  enum class Framework { None, D3D9, D3D11 };
  enum class Result { NotLoaded, NoFunction, Failed, Successful };

  static inline const TCHAR szModuleName[]    = TEXT("MirrorHook.asi");
  static inline HMODULE     hMirrorHookModule = nullptr;

  inline bool    IsMirrorHookLoaded() { return hMirrorHookModule != nullptr || (hMirrorHookModule = GetModuleHandle(szModuleName)) != nullptr; }
  inline FARPROC GetFn(const char* fnName) { return GetProcAddress(hMirrorHookModule, fnName); }

  inline Result Init(const Framework framework, const wchar_t* const szWindowTitle) {
    if (!IsMirrorHookLoaded()) return Result::NotLoaded;

    const auto fn = reinterpret_cast<bool(__stdcall*)(const Framework, const wchar_t* const)>(GetFn("MirrorHookInternals::InitWithWindowTitle"));
    return !fn ? Result::NoFunction : fn(framework, szWindowTitle) ? Result::Successful : Result::Failed;
  }
  inline Result Init(const Framework framework, const HWND& hWnd) {
    if (!IsMirrorHookLoaded()) return Result::NotLoaded;

    const auto fn = reinterpret_cast<bool(__stdcall*)(const Framework, const HWND&)>(GetFn("MirrorHookInternals::InitWithWindowHandle"));
    return !fn ? Result::NoFunction : fn(framework, hWnd) ? Result::Successful : Result::Failed;
  }
  inline Result Init(const Framework framework, IDirect3DDevice9** ppDevice) {
    if (!IsMirrorHookLoaded()) return Result::NotLoaded;

    const auto fn = reinterpret_cast<bool(__stdcall*)(const Framework, IDirect3DDevice9**)>(GetFn("MirrorHookInternals::InitWithDevicePointer"));
    return !fn ? Result::NoFunction : fn(framework, ppDevice) ? Result::Successful : Result::Failed;
  }

  namespace D3D9 {
    enum class Type { BeforeReset, AfterReset, Present, BeginScene, EndScene };

    inline Result AddExtension(const Type type, void(__stdcall* pExtension)(IDirect3DDevice9* pDevice)) {
      if (!IsMirrorHookLoaded()) return Result::NotLoaded;

      const auto fn = reinterpret_cast<bool(__stdcall*)(const Type, void(__stdcall*)(IDirect3DDevice9*))>(
        GetFn("MirrorHookInternals::D3D9Extender::details::AddExtension"));
      return !fn ? Result::NoFunction : fn(type, pExtension) ? Result::Successful : Result::Failed;
    }
    inline Result AddExtension(const Type type, void(__stdcall* pExtension)(IDirect3DDevice9* pDevice, void* pPresentationParameters)) {
      if (!IsMirrorHookLoaded()) return Result::NotLoaded;

      const auto fn = reinterpret_cast<bool(__stdcall*)(const Type, void(__stdcall*)(IDirect3DDevice9*, void*))>(
        GetFn("MirrorHookInternals::D3D9Extender::details::AddExtension"));
      return !fn ? Result::NoFunction : fn(type, pExtension) ? Result::Successful : Result::Failed;
    }
  } // namespace D3D9
  namespace D3D11 {
    enum class Type { Present };

    inline Result AddExtension(const Type type, void(__stdcall* pExtension)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags)) {
      if (!IsMirrorHookLoaded()) return Result::NotLoaded;

      const auto fn = reinterpret_cast<bool(__stdcall*)(const Type, void(__stdcall*)(IDXGISwapChain*, UINT, UINT))>(
        GetFn("MirrorHookInternals::D3D11Extender::details::AddExtension"));
      return !fn ? Result::NoFunction : fn(type, pExtension) ? Result::Successful : Result::Failed;
    }
  } // namespace D3D11
  namespace WndProc {
    constexpr LRESULT g_constIgnoreThisReturn = -1;

    inline Result AddExtension(LRESULT(__stdcall* pExtension)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)) {
      if (!IsMirrorHookLoaded()) return Result::NotLoaded;

      const auto fn = reinterpret_cast<void(__stdcall*)(LRESULT(__stdcall*)(HWND, UINT, WPARAM, LPARAM))>(
        GetFn("MirrorHookInternals::WndProcExtender::details::AddExtension"));
      if (fn) {
        fn(pExtension);
        return Result::Successful;
      }
      else {
        return Result::NoFunction;
      }
    }
    inline HWND GetWindowHandle() {
      if (!IsMirrorHookLoaded()) return nullptr;

      const auto fn = reinterpret_cast<HWND(__stdcall*)()>(GetFn("MirrorHookInternals::WndProcExtender::details::GetWindowHandle"));
      return !fn ? nullptr : fn();
    }
  } // namespace WndProc
} // namespace MirrorHook
#endif
