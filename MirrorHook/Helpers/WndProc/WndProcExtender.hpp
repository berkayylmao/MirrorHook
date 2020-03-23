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

#pragma once
#include "stdafx.h"

namespace MirrorHookInternals::WndProcExtender {
  std::mutex wndMutex;
  HWND       windowHandle = nullptr;
  WNDPROC    origWndProc  = nullptr;
  auto       extensions   = std::vector<WNDPROC>();

  LRESULT CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!extensions.empty()) {
      static LRESULT stickyRetVal = MirrorHook::WndProc::WndProcHook_NoReturn;
      for (WNDPROC wndProcExtender : extensions) {
        static LRESULT retVal;
        if (retVal = wndProcExtender(hWnd, uMsg, wParam, lParam) !=
                     MirrorHook::WndProc::WndProcHook_NoReturn)
          stickyRetVal = retVal;
      }
      if (stickyRetVal != MirrorHook::WndProc::WndProcHook_NoReturn) {
        LRESULT ret  = stickyRetVal;
        stickyRetVal = MirrorHook::WndProc::WndProcHook_NoReturn;
        return ret;
      }
    }
    return CallWindowProc(origWndProc, hWnd, uMsg, wParam, lParam);
  }

  void __stdcall AddExtension(LPVOID extensionAddress) {
#pragma ExportedFunction
    std::scoped_lock _lock(wndMutex);
    extensions.push_back(reinterpret_cast<WNDPROC>(extensionAddress));
  }
  HWND __stdcall GetWindowHandle() {
#pragma ExportedFunction
    return windowHandle;
  }

  void Init(HWND* pWindowHandle) {
    windowHandle = *pWindowHandle;
    origWndProc  = (WNDPROC)SetWindowLongPtr(windowHandle, GWLP_WNDPROC, (LONG_PTR)&hkWndProc);
  }
}  // namespace MirrorHookInternals::WndProcExtender
