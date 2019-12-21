/*
   MIT License

   Copyright (c) 2019 Berkay Yigit <berkay2578@gmail.com>
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
#include <WinDef.h>

namespace MirrorHook {
   enum class Game {
      MostWanted = 0,
      Carbon,
      UniversalD3D11
   };

   typedef bool(__stdcall* fn_PrepareFor)(Game gameType, const wchar_t* windowTitleName);
   typedef bool(__stdcall* fn_NoParam_ReturnsBool)();

   inline bool __stdcall PrepareFor(Game gameType, const wchar_t* windowTitleName) {
      return reinterpret_cast<fn_PrepareFor>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::PrepareFor"))(gameType, windowTitleName);
   }
   inline bool __stdcall IsReady() {
      return reinterpret_cast<fn_NoParam_ReturnsBool>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::IsReady"))();
   }
   inline bool __stdcall IsShowingInfoOverlay() {
      return reinterpret_cast<fn_NoParam_ReturnsBool>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::IsShowingInfoOverlay"))();
   }

   namespace DI8 {
      enum class DI8Device {
         Keyboard,
         Mouse
      };
      enum class DI8Extension {
         GetDeviceState
      };

   #ifndef __DINPUT_INCLUDED__
      typedef LPVOID LPDIRECTINPUT8A;
      typedef LPVOID LPDIRECTINPUTDEVICE8A;
   #endif
      typedef HRESULT(__stdcall* fnAddExtension)(DI8Device deviceType, DI8Extension extensionType, LPVOID extensionAddress);
      typedef LPDIRECTINPUT8A(__stdcall* fnGetDirectInput8A)();
      typedef LPDIRECTINPUTDEVICE8A(__stdcall* fnGetDirectInputDevice8A)(DI8Device deviceType);

      inline HRESULT __stdcall AddExtension(DI8Device deviceType, DI8Extension extensionType, LPVOID extensionAddress) {
         return reinterpret_cast<fnAddExtension>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::DI8Extender::AddExtension"))
            (deviceType, extensionType, extensionAddress);
      }
      inline LPDIRECTINPUT8A __stdcall GetDirectInput8A() {
         return reinterpret_cast<fnGetDirectInput8A>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::DI8Extender::GetDirectInput8A"))();
      }
      inline LPDIRECTINPUTDEVICE8A __stdcall GetDirectInputDevice8A(DI8Device deviceType) {
         return reinterpret_cast<fnGetDirectInputDevice8A>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::DI8Extender::GetDirectInputDevice8A"))
            (deviceType);
      }
      inline bool __stdcall IsReady() {
         return reinterpret_cast<fn_NoParam_ReturnsBool>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::DI8Extender::IsReady"))();
      }
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
      typedef HRESULT(__stdcall* fnSetTestCooperativeLevelExtension)(LPVOID extensionAddress);
      typedef HWND(__stdcall* fnGetWindowHandle)();
      typedef LPDIRECT3DDEVICE9(__stdcall* fnGetD3D9Device)();

      inline HRESULT __stdcall AddExtension(D3D9Extension extensionType, LPVOID extensionAddress) {
         return reinterpret_cast<fnAddExtension>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D9Extender::AddExtension"))
            (extensionType, extensionAddress);
      }
      inline HRESULT __stdcall SetTestCooperativeLevelExtension(LPVOID extensionAddress) {
         return reinterpret_cast<fnSetTestCooperativeLevelExtension>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D9Extender::SetTestCooperativeLevelExtension"))
            (extensionAddress);
      }
      inline HWND __stdcall GetWindowHandle() {
         return reinterpret_cast<fnGetWindowHandle>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D9Extender::GetWindowHandle"))();
      }
      inline LPDIRECT3DDEVICE9 __stdcall GetD3D9Device() {
         return reinterpret_cast<fnGetD3D9Device>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D9Extender::GetD3D9Device"))();
      }
      inline bool __stdcall IsReady() {
         return reinterpret_cast<fn_NoParam_ReturnsBool>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D9Extender::IsReady"))();
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
         return reinterpret_cast<fnAddExtension>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D11Extender::AddExtension"))
            (extensionType, extensionAddress);
      }
      inline ID3D11Device* __stdcall GetD3D9Device() {
         return reinterpret_cast<fnGetD3D11Device>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D11Extender::GetD3D11Device"))();
      }
      inline ID3D11DeviceContext* __stdcall GetD3D11DeviceContext() {
         return reinterpret_cast<fnGetD3D11DeviceContext>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D11Extender::GetD3D11DeviceContext"))();

      }
      inline bool __stdcall IsReady() {
         return reinterpret_cast<fn_NoParam_ReturnsBool>(GetProcAddress(GetModuleHandle(TEXT("MirrorHook.dll")), "MirrorHookInternals::D3D11Extender::IsReady"))();
      }
   }

}
#endif