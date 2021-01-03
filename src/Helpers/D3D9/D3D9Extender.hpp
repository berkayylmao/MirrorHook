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

// D3D9
#pragma warning(push, 0)
#include <d3d9.h>
#pragma warning(pop)
using Reset_t           = HRESULT(__stdcall*)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
using BeginScene_t      = HRESULT(__stdcall*)(LPDIRECT3DDEVICE9);
using EndScene_t        = HRESULT(__stdcall*)(LPDIRECT3DDEVICE9);
using BeginStateBlock_t = HRESULT(__stdcall*)(LPDIRECT3DDEVICE9);

namespace MirrorHookInternals::D3D9Extender {
  enum class D3D9Extension { BeginScene, EndScene, BeforeReset, AfterReset };

  namespace details {
    static inline std::mutex                InternalMutex;
    static inline std::vector<BeginScene_t> BeginSceneExts;
    static inline std::vector<EndScene_t>   EndSceneExts;
    static inline std::vector<Reset_t>      BeforeResetExts;
    static inline std::vector<Reset_t>      AfterRestExts;

#pragma region Hooks
    // detours
    static inline std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, BeginScene_t>      BeginSceneDetour;
    static inline std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, EndScene_t>        EndSceneDetour;
    static inline std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, Reset_t>           ResetDetour;
    static inline std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, BeginStateBlock_t> BeginStateBlockDetour;
    // waiting list (for BeginScene)
    static inline std::vector<EndScene_t> BeginSceneWaitingExts;

    inline auto __stdcall hkBeginScene(LPDIRECT3DDEVICE9 pDevice) -> HRESULT {
      // call original
      BeginSceneDetour.first->Undetour();
      const auto _ret = BeginSceneDetour.second(pDevice);
      BeginSceneDetour.first->Detour();
      // clear waiting list
      for (const auto& _ext : BeginSceneWaitingExts) EndSceneExts.push_back(_ext);
      BeginSceneWaitingExts.clear();
      // call extensions
      for (const auto& _ext : BeginSceneExts) _ext(pDevice);
      // return original
      return _ret;
    }

    inline auto __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) -> HRESULT {
      // cal original
      EndSceneDetour.first->Undetour();
      const auto _ret = EndSceneDetour.second(pDevice);
      EndSceneDetour.first->Detour();
      // call extensions
      for (const auto& _ext : EndSceneExts) _ext(pDevice);
      // return original
      return _ret;
    }

    inline auto __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) -> HRESULT {
      // call extensions
      for (const auto& _ext : BeforeResetExts) _ext(pDevice, pPresentationParameters);
      // call original
      ResetDetour.first->Undetour();
      const auto _ret = ResetDetour.second(pDevice, pPresentationParameters);
      ResetDetour.first->Detour();
      // call extensions
      for (const auto& _ext : AfterRestExts) _ext(pDevice, pPresentationParameters);
      // return original
      return _ret;
    }

    inline auto __stdcall hkBeginStateBlock(LPDIRECT3DDEVICE9 pDevice) -> HRESULT {
      // reverse all detours
      BeginSceneDetour.first->Undetour();
      BeginSceneDetour.first.reset();
      EndSceneDetour.first->Undetour();
      EndSceneDetour.first.reset();
      ResetDetour.first->Undetour();
      ResetDetour.first.reset();
      BeginStateBlockDetour.first->Undetour();
      BeginStateBlockDetour.first.reset();
      // call original
      const auto _ret = BeginStateBlockDetour.second(pDevice);
      // detour all
      auto* _vtDevice       = *reinterpret_cast<std::uintptr_t**>(pDevice);
      BeginStateBlockDetour = {MemoryEditor::Get().Detour(_vtDevice[60], reinterpret_cast<std::uintptr_t>(&hkBeginStateBlock)),
                               reinterpret_cast<BeginStateBlock_t>(_vtDevice[60])};
      BeginSceneDetour      = {MemoryEditor::Get().Detour(_vtDevice[41], reinterpret_cast<std::uintptr_t>(&hkBeginScene)),
                          reinterpret_cast<BeginScene_t>(_vtDevice[41])};
      EndSceneDetour        = {MemoryEditor::Get().Detour(_vtDevice[42], reinterpret_cast<std::uintptr_t>(&hkEndScene)),
                        reinterpret_cast<EndScene_t>(_vtDevice[42])};
      ResetDetour = {MemoryEditor::Get().Detour(_vtDevice[16], reinterpret_cast<std::uintptr_t>(&hkReset)), reinterpret_cast<Reset_t>(_vtDevice[16])};
      // return original
      return _ret;
    }
#pragma endregion
#pragma region Exported
    [[maybe_unused]] auto __stdcall AddExtension(const D3D9Extension type, void(__stdcall* pExtension)(LPDIRECT3DDEVICE9)) -> bool {
#pragma __EXPORTED_FUNCTION__
      if (!pExtension) return false;
      std::scoped_lock<std::mutex> _l(InternalMutex);

      switch (type) {
        case D3D9Extension::BeginScene:
          BeginSceneExts.push_back(reinterpret_cast<BeginScene_t>(pExtension));
          return true;
        case D3D9Extension::EndScene:
          BeginSceneWaitingExts.push_back(reinterpret_cast<EndScene_t>(pExtension));
          return true;
        case D3D9Extension::BeforeReset:
          BeforeResetExts.push_back(reinterpret_cast<Reset_t>(pExtension));
          return true;
        case D3D9Extension::AfterReset:
          AfterRestExts.push_back(reinterpret_cast<Reset_t>(pExtension));
          return true;
      }
      return false;
    }
#pragma endregion

    inline void DoHook(std::uintptr_t vtDevice[]) {
      BeginStateBlockDetour = {MemoryEditor::Get().Detour(vtDevice[60], reinterpret_cast<std::uintptr_t>(&hkBeginStateBlock)),
                               reinterpret_cast<BeginStateBlock_t>(vtDevice[60])};
      BeginSceneDetour      = {MemoryEditor::Get().Detour(vtDevice[41], reinterpret_cast<std::uintptr_t>(&hkBeginScene)),
                          reinterpret_cast<BeginScene_t>(vtDevice[41])};
      EndSceneDetour        = {MemoryEditor::Get().Detour(vtDevice[42], reinterpret_cast<std::uintptr_t>(&hkEndScene)),
                        reinterpret_cast<EndScene_t>(vtDevice[42])};
      ResetDetour = {MemoryEditor::Get().Detour(vtDevice[16], reinterpret_cast<std::uintptr_t>(&hkReset)), reinterpret_cast<Reset_t>(vtDevice[16])};
    }
  } // namespace details

#pragma warning(push)
#pragma warning(disable : 28182) // dereferencing null pointer

  inline auto Init(LPDIRECT3DDEVICE9* ppDevice) -> bool {
    // check if target is running d3d9
    if (!GetModuleHandle(TEXT("d3d9.dll"))) return false;
    // local storage
    LPDIRECT3DDEVICE9 _pDevice;

    // capture device
    {
      // wait for device init
      while (!*ppDevice) std::this_thread::sleep_for(std::chrono::milliseconds(100));
      _pDevice = *ppDevice;
      // hook device
      details::DoHook(*reinterpret_cast<std::uintptr_t**>(_pDevice));
    }

    // capture wndproc
    {
      // get creation params (to hook wndproc)
      D3DDEVICE_CREATION_PARAMETERS _cParams;
      _pDevice->GetCreationParameters(&_cParams);
      // hook wndproc
      WndProcExtender::Init(_cParams.hFocusWindow);
    }

    return true;
  }
#pragma warning(pop)

  inline auto Init(const HWND& hWindow) -> bool {
    // check if target is running d3d9
    auto* _hLib = GetModuleHandle(TEXT("d3d9.dll"));
    if (!_hLib) return false;
    // local storage
    LPDIRECT3DDEVICE9 _pDevice{nullptr};

    // capture device
    {
      // try retrieve device from Direct3DCreate9
      {
        D3DPRESENT_PARAMETERS _params{};
        {
          memset(&_params, NULL, sizeof(D3DPRESENT_PARAMETERS));

          _params.SwapEffect    = D3DSWAPEFFECT_DISCARD;
          _params.hDeviceWindow = hWindow;
          _params.Windowed      = FALSE;
        }

        auto* _pD3D = reinterpret_cast<LPDIRECT3D9(__stdcall*)(UINT)>(GetProcAddress(_hLib, "Direct3DCreate9"))(D3D_SDK_VERSION);
        if (FAILED(_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &_params, &_pDevice))) {
          _params.Windowed = TRUE;
          if (FAILED(_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &_params, &_pDevice))) {
            _pD3D->Release();
            return false;
          }
        }

        // release d3d
        _pD3D->Release();
      }

      // hook dummy device vtable
      details::DoHook(*reinterpret_cast<std::uintptr_t**>(_pDevice));
    }

    // capture WndProc
    {
      // get creation params (to hook wndproc)
      D3DDEVICE_CREATION_PARAMETERS _cParams;
      _pDevice->GetCreationParameters(&_cParams);
      // hook wndproc
      WndProcExtender::Init(_cParams.hFocusWindow);
    }

    // release dummy device
    _pDevice->Release();

    return true;
  }

  inline auto Init(const wchar_t* const szWindowTitle) -> bool {
    if (auto* _hWindow = FindWindowW(nullptr, szWindowTitle)) return Init(_hWindow);
    return false;
  }
} // namespace MirrorHookInternals::D3D9Extender
