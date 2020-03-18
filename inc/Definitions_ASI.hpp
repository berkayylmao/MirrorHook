/*
   The MIT License (MIT)

   Copyright (c) 2020 Berkay Yigit <berkaytgy@gmail.com>
       Copyright holder detail: Nickname(s) used by the copyright holder: 'berkay2578', 'berkayylmao'.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#pragma once
#ifndef __MIRRORHOOK_INCLUDED__
#define __MIRRORHOOK_INCLUDED__

#pragma warning(push, 0)
#include <WinDef.h>
#pragma warning(pop)

namespace MirrorHook {
   enum class Framework {
      None,
      D3D9,
      D3D11
   };

   typedef bool(__stdcall* fn_PrepareFor_WindowTitle)(Framework, const TCHAR* const);
   typedef bool(__stdcall* fn_PrepareFor_WindowHandle)(Framework, HWND*);
   typedef Framework(__stdcall* fn_GetInstalledFrameWork)();
   typedef bool(__stdcall* fn_NoParam_ReturnsBool)();

   inline bool __stdcall PrepareFor(Framework frameworkType, const TCHAR const* windowTitleName) {
      return reinterpret_cast<fn_PrepareFor_WindowTitle>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::PrepareForWithWindowTitleName"))(frameworkType, windowTitleName);
   }
   inline bool __stdcall PrepareFor(Framework frameworkType, HWND* pWindowHandle) {
      return reinterpret_cast<fn_PrepareFor_WindowHandle>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::PrepareForWithWindowHandle"))(frameworkType, pWindowHandle);
   }

   inline Framework __stdcall GetInstalledFramework() {
      return reinterpret_cast<fn_GetInstalledFrameWork>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::GetInstalledFramework"))();
   }
   inline bool __stdcall IsShowingInfoOverlay() {
      return reinterpret_cast<fn_NoParam_ReturnsBool>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::IsShowingInfoOverlay"))();
   }
   inline bool __stdcall IsReady() {
      return reinterpret_cast<fn_NoParam_ReturnsBool>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::IsReady"))();
   }
   inline bool __stdcall WasAutoInitSuccessful() {
      return reinterpret_cast<fn_NoParam_ReturnsBool>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::IsReady_WaitAutoInit"))();
   }

   namespace D3D9 {
      enum class D3D9Extension {
         BeginScene,
         EndScene,
         BeforeReset,
         AfterReset
      };

   #ifndef _D3D9_H_
      typedef LPVOID LPDIRECT3DDEVICE9;
   #endif
      typedef HRESULT(__stdcall* fnAddExtension)(D3D9Extension extenderType, LPVOID extensionAddress);
      typedef LPDIRECT3DDEVICE9(__stdcall* fnGetD3D9Device)();

      inline HRESULT __stdcall AddExtension(D3D9Extension extensionType, LPVOID extensionAddress) {
         return reinterpret_cast<fnAddExtension>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D9Extender::AddExtension"))
            (extensionType, extensionAddress);
      }
      inline LPDIRECT3DDEVICE9 __stdcall GetD3D9Device() {
         return reinterpret_cast<fnGetD3D9Device>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D9Extender::GetD3D9Device"))();
      }
      inline bool __stdcall IsReady() {
         return reinterpret_cast<fn_NoParam_ReturnsBool>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D9Extender::IsReady"))();
      }
   }
   namespace D3D11 {
      enum class D3D11Extension {
         Present
      };

   #ifndef __d3d11_h__
      typedef LPVOID ID3D11Device;
      typedef LPVOID ID3D11DeviceContext;
   #endif
      typedef HRESULT(__stdcall* fnAddExtension)(D3D11Extension extenderType, LPVOID extensionAddress);
      typedef ID3D11Device*(__stdcall* fnGetD3D11Device)();
      typedef ID3D11DeviceContext*(__stdcall* fnGetD3D11DeviceContext)();

      inline HRESULT __stdcall AddExtension(D3D11Extension extensionType, LPVOID extensionAddress) {
         return reinterpret_cast<fnAddExtension>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D11Extender::AddExtension"))
            (extensionType, extensionAddress);
      }
      inline ID3D11Device* __stdcall GetD3D9Device() {
         return reinterpret_cast<fnGetD3D11Device>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D11Extender::GetD3D11Device"))();
      }
      inline ID3D11DeviceContext* __stdcall GetD3D11DeviceContext() {
         return reinterpret_cast<fnGetD3D11DeviceContext>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D11Extender::GetD3D11DeviceContext"))();

      }
      inline bool __stdcall IsReady() {
         return reinterpret_cast<fn_NoParam_ReturnsBool>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::D3D11Extender::IsReady"))();
      }
   }
   namespace WndProc {
      constexpr LRESULT WndProcHook_NoReturn = -1;
      typedef HRESULT(__stdcall* fnAddExtension)(LPVOID extensionAddress);
      typedef HWND(__stdcall* fnGetWindowHandle)();

      inline HRESULT __stdcall AddExtension(LPVOID extensionAddress) {
         return reinterpret_cast<fnAddExtension>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::WndProcExtender::AddExtension"))
            (extensionAddress);
      }
      inline HWND __stdcall GetWindowHandle() {
         return reinterpret_cast<fnGetWindowHandle>(_Notnull_ GetProcAddress(GetModuleHandle(TEXT("MirrorHook.asi")), "MirrorHookInternals::WndProcExtender::GetWindowHandle"))();
      }
   }
}
#endif