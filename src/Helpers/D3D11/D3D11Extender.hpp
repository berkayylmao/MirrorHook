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
#include "Helpers/MemoryEditor/MemoryEditor.hpp"
#include "Helpers/WndProc/WndProcExtender.hpp"

// D3D11
#pragma warning(push, 0)
#include <D3D11.h>
#pragma warning(pop)
using D3D11Present_t = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);

namespace MirrorHookInternals::D3D11Extender {
  enum class D3D11Extension { Present };

  namespace details {
    static inline std::mutex                InternalMutex;
    static inline std::list<D3D11Present_t> PresentExts;

#pragma region Hooks
    static inline std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, D3D11Present_t> PresentDetour;

    static auto __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) -> HRESULT {
      // call original
      PresentDetour.first->Undetour();
      const auto _ret = PresentDetour.second(pSwapChain, SyncInterval, Flags);
      PresentDetour.first->Detour();
      // call extensions
      for (const auto& _ext : PresentExts) _ext(pSwapChain, SyncInterval, Flags);
      // return original
      return _ret;
    }
#pragma endregion
#pragma region Exported
    [[maybe_unused]] auto __stdcall AddExtension(const D3D11Extension type, void(__stdcall* pExtension)(IDXGISwapChain*, UINT, UINT)) -> bool {
#pragma __EXPORTED_FUNCTION__
      if (!pExtension) return false;
      std::scoped_lock<std::mutex> _l(InternalMutex);

      if (type == D3D11Extension::Present) {
        PresentExts.push_back(reinterpret_cast<D3D11Present_t>(pExtension));
        return true;
      }
      return false;
    }
#pragma endregion

    inline void DoHook(std::uintptr_t vtDevice[]) {
      PresentDetour = {MemoryEditor::Get().Detour(vtDevice[8], reinterpret_cast<std::uintptr_t>(&hkPresent)),
                       reinterpret_cast<D3D11Present_t>(vtDevice[8])};
    }
  } // namespace details

  inline auto Init(const HWND& hWindow) -> bool {
    // check if target is running d3d11
    auto* _hLib = GetModuleHandle(TEXT("d3d11.dll"));
    if (!_hLib) return false;

    // capture device
    {
      // local storage
      ID3D11Device*        _pFakeDevice{nullptr};
      ID3D11DeviceContext* _pFakeDeviceCtx{nullptr};
      IDXGISwapChain*      _pFakeSwapChain{nullptr};

      // try retrieve device from D3D11CreateDeviceAndSwapChain
      {
        D3D_FEATURE_LEVEL    _levels[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
        D3D_FEATURE_LEVEL    _obtainedLevel;
        DXGI_SWAP_CHAIN_DESC _sd;
        {
          _sd.BufferCount       = 1;
          _sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
          _sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
          _sd.OutputWindow      = hWindow;
          _sd.SampleDesc.Count  = 1;
          _sd.Windowed          = TRUE;
          _sd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;
          _sd.Flags             = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        }

        if (FAILED(reinterpret_cast<HRESULT(__stdcall*)(IDXGIAdapter*,
                                                        D3D_DRIVER_TYPE,
                                                        HMODULE,
                                                        UINT,
                                                        const D3D_FEATURE_LEVEL*,
                                                        UINT,
                                                        UINT,
                                                        const DXGI_SWAP_CHAIN_DESC*,
                                                        IDXGISwapChain**,
                                                        ID3D11Device**,
                                                        D3D_FEATURE_LEVEL*,
                                                        ID3D11DeviceContext**)>(GetProcAddress(_hLib, "D3D11CreateDeviceAndSwapChain"))(
              nullptr,
              D3D_DRIVER_TYPE_HARDWARE,
              nullptr,
              0,
              _levels,
              sizeof(_levels) / sizeof(D3D_FEATURE_LEVEL),
              D3D11_SDK_VERSION,
              &_sd,
              &_pFakeSwapChain,
              &_pFakeDevice,
              &_obtainedLevel,
              &_pFakeDeviceCtx)))
          return false;
      }

      // hook dummy device vtable
      { details::DoHook(*reinterpret_cast<std::uintptr_t**>(_pFakeSwapChain)); }

      // release dummy device
      {
        _pFakeSwapChain->Release();
        _pFakeDevice->Release();
        _pFakeDeviceCtx->Release();
      }
    }

    // capture WndProc
    { WndProcExtender::Init(hWindow); }
    return true;
  }

  inline auto Init(const wchar_t* const szWindowTitle) -> bool {
    if (auto* _hWindow = FindWindowW(nullptr, szWindowTitle)) return Init(_hWindow);
    return false;
  }
} // namespace MirrorHookInternals::D3D11Extender
