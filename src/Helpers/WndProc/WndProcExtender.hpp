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

#pragma once
#include "pch.h"

namespace MirrorHookInternals::WndProcExtender {
  namespace details {
    static inline std::mutex         InternalMutex;
    static inline HWND               TargetWindowHandle;
    static inline std::list<WNDPROC> WndProcExts;

#pragma region Hooks
    static inline WNDPROC OriginalWndProc;

    inline auto CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT {
      // local storage
      static LRESULT _retVal = -1;
      // call extensions
      for (const auto& ext : WndProcExts) _retVal &= ext(hWnd, uMsg, wParam, lParam);
      // check for custom retVal
      if (_retVal != -1) {
        const auto _ret = _retVal;
        _retVal         = -1;

        // return custom retVal
        return _ret;
      }

      // return original
      return CallWindowProc(OriginalWndProc, hWnd, uMsg, wParam, lParam);
    }
#pragma endregion
#pragma region Exported
    [[maybe_unused]]  void __stdcall AddExtension(void(__stdcall* pExtension)(HWND, UINT, WPARAM, LPARAM)) {
#pragma __EXPORTED_FUNCTION__
      if (!pExtension) return;
      std::scoped_lock<std::mutex> _l(InternalMutex);

      WndProcExts.push_back(reinterpret_cast<WNDPROC>(pExtension));
    }

    [[maybe_unused]] auto __stdcall GetWindowHandle() -> HWND {
#pragma __EXPORTED_FUNCTION__
      return TargetWindowHandle;
    }
#pragma endregion

    inline void DoHook(const HWND& hWindow) {
      if (!OriginalWndProc) {
        TargetWindowHandle = hWindow;
        OriginalWndProc    = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&hkWndProc)));
      }
    }
  } // namespace details

  inline void Init(const HWND& hWindow) { details::DoHook(hWindow); }
} // namespace MirrorHookInternals::WndProcExtender
