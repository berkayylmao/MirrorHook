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
#include "Helpers/D3D11/D3D11Def.h"
#include "Helpers/MemoryEditor/MemoryEditor.hpp"
#include "Helpers/WndProc/WndProcExtender.hpp"

namespace MirrorHookInternals::D3D11Extender {
  enum class D3D11Extension { Present };

  std::mutex                    mMutex;
  std::vector<D3D11::Present_t> mPresentExts;

#pragma region Hooks
  D3D11::Present_t                                  origPresent = nullptr;
  std::unique_ptr<MemoryEditor::Editor::DetourInfo> detourInfo  = nullptr;

  HRESULT __stdcall hkPresent(D3D11::IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
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
      mPresentExts.push_back(reinterpret_cast<D3D11::Present_t>(pExtension));
      return true;
    }

    return false;
  }
#pragma endregion

  bool Init(HWND& hWindow) {
    auto hLib = GetModuleHandle(TEXT("d3d11.dll"));
    if (!hLib) return false;

    INT                         levels[2] = {0xa100, 0xb000};
    INT                         obtainedLevel;
    D3D11::DXGI_SWAP_CHAIN_DESC sd;
    {
      sd.BufferCount       = 1;
      sd.BufferDesc.Format = 28;
      sd.BufferUsage       = 32;
      sd.OutputWindow      = hWindow;
      sd.SampleDesc.Count  = 1;
      sd.Windowed          = TRUE;
      sd.SwapEffect        = 0;
      sd.Flags             = 2;
    }

    D3D11::ID3D11Device*        pFakeDevice;
    D3D11::ID3D11DeviceContext* pFakeDeviceCtx;
    D3D11::IDXGISwapChain*      pFakeSwapChain;
    if (FAILED(reinterpret_cast<HRESULT(__stdcall*)(void*, INT, HMODULE, UINT, const INT*, UINT, UINT,
                                                    const D3D11::DXGI_SWAP_CHAIN_DESC*, D3D11::IDXGISwapChain**,
                                                    D3D11::ID3D11Device**, INT*, D3D11::ID3D11DeviceContext**)>(
            GetProcAddress(hLib, "D3D11CreateDeviceAndSwapChain"))(nullptr, 1, nullptr, 0, levels, _countof(levels), 7,
                                                                   &sd, &pFakeSwapChain, &pFakeDevice, &obtainedLevel,
                                                                   &pFakeDeviceCtx)))
      return false;

    auto* vtDevice = *(std::uintptr_t**)pFakeSwapChain;
    origPresent    = reinterpret_cast<D3D11::Present_t>(vtDevice[8]);
    detourInfo     = std::move(MemoryEditor::Get().Detour(vtDevice[8], reinterpret_cast<std::uintptr_t>(&hkPresent)));

    pFakeSwapChain->Release();
    pFakeDevice->Release();
    pFakeDeviceCtx->Release();

    pFakeSwapChain = nullptr;
    pFakeDevice    = nullptr;
    pFakeDeviceCtx = nullptr;

    WndProcExtender::Init(hWindow);
    return true;
  }
  bool Init(const wchar_t* const szWindowTitle) {
    if (auto hWindow = FindWindowW(0, szWindowTitle)) return Init(hWindow);
    return false;
  }
}  // namespace MirrorHookInternals::D3D11Extender
