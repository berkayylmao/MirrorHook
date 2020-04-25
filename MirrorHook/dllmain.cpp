/*
   The MIT License (MIT)

   Copyright (c) 2020 Berkay Yigit <berkaytgy@gmail.com>
       Copyright holder detail: Nickname(s) used by the copyright holder: 'berkay2578',
   'berkayylmao'.

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
#include "Helpers/D3D9/D3D9Extender.hpp"
#include "Helpers/D3D11/D3D11Extender.hpp"

namespace MirrorHookInternals {
  std::mutex            initMutex;
  bool                  bStopWaitingAutoInit = false;
  MirrorHook::Framework installedFramework   = MirrorHook::Framework::None;

  bool __stdcall IsReady() {
#pragma ExportedFunction
    return installedFramework != MirrorHook::Framework::None;
  }
  bool __stdcall IsReady_WaitAutoInit() {
#pragma ExportedFunction
    while (!bStopWaitingAutoInit) Sleep(750);

    return installedFramework != MirrorHook::Framework::None;
  }

  bool __stdcall PrepareForWithWindowTitleName(MirrorHook::Framework requestedFrameworkType,
                                               const TCHAR* const    windowTitleName) {
#pragma ExportedFunction
    initMutex.lock();
    if (windowTitleName) {
      switch (requestedFrameworkType) {
        case MirrorHook::Framework::D3D9: {
          if (D3D9Extender::Init(windowTitleName)) {
            installedFramework   = MirrorHook::Framework::D3D9;
            bStopWaitingAutoInit = true;
          }
        } break;
        case MirrorHook::Framework::D3D11: {
          if (D3D11Extender::Init(windowTitleName)) {
            installedFramework   = MirrorHook::Framework::D3D11;
            bStopWaitingAutoInit = true;
          }
        } break;
      }
    }
    initMutex.unlock();
    return IsReady();
  }
  bool __stdcall PrepareForWithWindowHandle(MirrorHook::Framework requestedFrameworkType,
                                            HWND*                 pWindowHandle) {
#pragma ExportedFunction
    initMutex.lock();
    if (pWindowHandle) {
      switch (requestedFrameworkType) {
        case MirrorHook::Framework::D3D9: {
          if (D3D9Extender::Init(pWindowHandle)) {
            installedFramework   = MirrorHook::Framework::D3D9;
            bStopWaitingAutoInit = true;
          }
        } break;
        case MirrorHook::Framework::D3D11: {
          if (D3D11Extender::Init(pWindowHandle)) {
            installedFramework   = MirrorHook::Framework::D3D11;
            bStopWaitingAutoInit = true;
          }
        } break;
      }
    }
    initMutex.unlock();
    return IsReady();
  }
  bool __stdcall PrepareForWithDevicePointer(MirrorHook::Framework requestedFrameworkType,
                                             LPVOID*               ppDevice) {
#pragma ExportedFunction
    initMutex.lock();
    if (ppDevice) {
      switch (requestedFrameworkType) {
        case MirrorHook::Framework::D3D9: {
          if (D3D9Extender::Init(reinterpret_cast<LPDIRECT3DDEVICE9*>(ppDevice))) {
            installedFramework   = MirrorHook::Framework::D3D9;
            bStopWaitingAutoInit = true;
          }
        } break;
      }
    }
    initMutex.unlock();
    return IsReady();
  }
}  // namespace MirrorHookInternals

#pragma region Try Auto - Init
uint32_t           maxTryCount = 5;
uint32_t           curTryCount = 0;

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
  DWORD pId;
  GetWindowThreadProcessId(hwnd, &pId);
  if (GetCurrentProcessId() != pId)
    return TRUE;
  else {
    *(HWND*)lParam = hwnd;
    return FALSE;
  }
}

DWORD WINAPI Init(LPVOID) {  // try to auto init
  HWND hWnd = nullptr;
  while (!hWnd) {
    if (curTryCount == maxTryCount) break;

    EnumWindows(EnumWindowsCallback, (LPARAM)&hWnd);
    Sleep(1000);
    curTryCount++;
  }

  if (MirrorHookInternals::installedFramework == MirrorHook::Framework::None &&
      hWnd) {  // in case MirrorHook was set up externally
    if (GetModuleHandle(TEXT("d3d9.dll"))) {
      MirrorHookInternals::PrepareForWithWindowHandle(MirrorHook::Framework::D3D9, &hWnd);
    }
    if (GetModuleHandle(TEXT("d3d11.dll"))) {
      MirrorHookInternals::PrepareForWithWindowHandle(MirrorHook::Framework::D3D11, &hWnd);
    }
  }

  MirrorHookInternals::bStopWaitingAutoInit = true;
  return TRUE;
}
#pragma endregion

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
  if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    if (auto t = CreateThread(NULL, 0, &Init, NULL, 0, NULL)) CloseHandle(t);
  }
  return TRUE;
}