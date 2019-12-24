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

#include "stdafx.h"
#include "Helpers/Internal/DI8/DI8Extender.hpp"
#include "Helpers/Internal/D3D9/D3D9Extender.hpp"
#include "Helpers/Internal/D3D11/D3D11Extender.hpp"

namespace MirrorHookInternals {
   bool                  isInit = false;
   MirrorHook::Framework installedFramework;

   bool __stdcall PrepareForWithWindowTitleName(MirrorHook::Framework frameworkType, const TCHAR* const windowTitleName = nullptr) {
   #pragma ExportedFunction
      if (!isInit) {
         installedFramework = frameworkType;
         switch (frameworkType) {
            case MirrorHook::Framework::NFSMostWanted:
            {
               Memory::Init((DWORD_PTR)GetModuleHandleW(NULL));
               return DI8Extender::Init(0x582D14) && D3D9Extender::Init(windowTitleName); // Memory::makeAbsolute(0x582BDC);
            }
            break;
            case MirrorHook::Framework::NFSCarbon:
            {
               Memory::Init((DWORD_PTR)GetModuleHandleW(NULL));
               return DI8Extender::Init(0x71F5CC) && D3D9Extender::Init(windowTitleName); // Memory::makeAbsolute(0x6B0ABC);
            }
            case MirrorHook::Framework::UniversalD3D9:
            {
               Memory::Init();
               return D3D9Extender::Init(windowTitleName);
            }
            case MirrorHook::Framework::UniversalD3D11:
            {
               Memory::Init();
               return D3D11Extender::Init(windowTitleName);
            }
         }
         return false;
      }
      return false;
   }
   bool __stdcall PrepareForWithWindowHandle(MirrorHook::Framework frameworkType, HWND* pWindowHandle) {
   #pragma ExportedFunction
      if (!isInit || pWindowHandle) {
         installedFramework = frameworkType;
         switch (frameworkType) {
            case MirrorHook::Framework::NFSMostWanted:
            {
               Memory::Init((DWORD_PTR)GetModuleHandleW(NULL));
               return DI8Extender::Init(0x582D14) && D3D9Extender::Init(pWindowHandle); // Memory::makeAbsolute(0x582BDC);
            }
            break;
            case MirrorHook::Framework::NFSCarbon:
            {
               Memory::Init((DWORD_PTR)GetModuleHandleW(NULL));
               return DI8Extender::Init(0x71F5CC) && D3D9Extender::Init(pWindowHandle); // Memory::makeAbsolute(0x6B0ABC);
            }
            case MirrorHook::Framework::UniversalD3D9:
            {
               Memory::Init();
               return D3D9Extender::Init(pWindowHandle);
            }
            case MirrorHook::Framework::UniversalD3D11:
            {
               Memory::Init();
               return D3D11Extender::Init(pWindowHandle);
            }
         }
         return false;
      }
      return false;
   }
   bool __stdcall IsShowingInfoOverlay() {
   #pragma ExportedFunction
      if (installedFramework == MirrorHook::Framework::UniversalD3D11)
         return (D3D11Extender::infoOverlayFrame < D3D11Extender::infoOverlayFrame_MaxFrame);
      else
         return (D3D9Extender::infoOverlayFrame < D3D9Extender::infoOverlayFrame_MaxFrame);
   }
   bool __stdcall IsReady() {
   #pragma ExportedFunction
      return isInit;
   }

   BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
      if (ul_reason_for_call == DLL_PROCESS_ATTACH)
         DisableThreadLibraryCalls(hModule);
      return TRUE;
   }
}