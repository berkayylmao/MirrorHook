// clang-format off
//
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
// d3d1
#pragma warning(push, 0)
#include <d3d11.h>
#pragma warning(pop)
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

namespace MirrorHookInternals::D3D11Extender {
  enum class D3D11Extension { Present };

  std::mutex             mMutex;
  std::vector<Present_t> mPresentExts;

#pragma region Hooks
  Present_t                                         origPresent = nullptr;
  std::unique_ptr<MemoryEditor::Editor::DetourInfo> detourInfo  = nullptr;

  HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    for (const auto& ext : mPresentExts)
      if (ext) ext(pSwapChain, SyncInterval, Flags);

    detourInfo->Undetour();
    auto ret = origPresent(pSwapChain, SyncInterval, Flags);
    detourInfo->Detour();
    return ret;
  }
#pragma endregion
#pragma region Exported
  bool __stdcall AddExtension(D3D11Extension type, void* pExtension) {
#pragma ExportedFunction
    std::scoped_lock<std::mutex> _l(mMutex);
    if (type == D3D11Extension::Present) {
      mPresentExts.push_back(reinterpret_cast<Present_t>(pExtension));
      return true;
    }

    return false;
  }
#pragma endregion

  bool Init(HWND& hWindow) {
    auto hLib = GetModuleHandle(TEXT("d3d11.dll"));
    if (!hLib) return false;

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

    ID3D11Device*        _pFakeDevice;
    ID3D11DeviceContext* _pFakeDeviceCtx;
    IDXGISwapChain*      _pFakeSwapChain;
    if (FAILED(reinterpret_cast<HRESULT(__stdcall*)(
                   IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT, UINT,
                   const DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, ID3D11Device**, D3D_FEATURE_LEVEL*,
                   ID3D11DeviceContext**)>(GetProcAddress(hLib, "D3D11CreateDeviceAndSwapChain"))(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, _levels, sizeof(_levels) / sizeof(D3D_FEATURE_LEVEL),
            D3D11_SDK_VERSION, &_sd, &_pFakeSwapChain, &_pFakeDevice, &_obtainedLevel, &_pFakeDeviceCtx)))
      return false;

    auto* vtDevice = *(std::uintptr_t**)_pFakeSwapChain;
    origPresent    = reinterpret_cast<Present_t>(vtDevice[8]);
    detourInfo     = std::move(MemoryEditor::Get().Detour(vtDevice[8], reinterpret_cast<std::uintptr_t>(&hkPresent)));

    _pFakeSwapChain->Release();
    _pFakeDevice->Release();
    _pFakeDeviceCtx->Release();

    _pFakeSwapChain = nullptr;
    _pFakeDevice    = nullptr;
    _pFakeDeviceCtx = nullptr;

    WndProcExtender::Init(hWindow);
    return true;
  }
  bool Init(const wchar_t* const szWindowTitle) {
    if (auto hWindow = FindWindowW(0, szWindowTitle)) return Init(hWindow);
    return false;
  }
}  // namespace MirrorHookInternals::D3D11Extender
