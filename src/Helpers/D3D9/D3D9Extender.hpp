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
#include "Helpers/D3D9/D3D9Def.h"
#include "Helpers/MemoryEditor/MemoryEditor.hpp"
#include "Helpers/WndProc/WndProcExtender.hpp"

namespace MirrorHookInternals::D3D9Extender {
  enum class D3D9Extension { BeginScene, EndScene, BeforeReset, AfterReset };

  std::mutex                      mMutex;
  std::vector<D3D9::BeginScene_t> mBeginSceneExts;
  std::vector<D3D9::EndScene_t>   mEndSceneExts;
  std::vector<D3D9::Reset_t>      mBefResetExts;
  std::vector<D3D9::Reset_t>      mAftResetExts;

#pragma region Hooks
  // detours
  std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, D3D9::BeginScene_t>      mBeginScene;
  std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, D3D9::EndScene_t>        mEndScene;
  std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, D3D9::Reset_t>           mReset;
  std::pair<std::unique_ptr<MemoryEditor::Editor::DetourInfo>, D3D9::BeginStateBlock_t> mBeginStateBlock;

  HRESULT __stdcall hkBeginScene(D3D9::LPDIRECT3DDEVICE9 pDevice) {
    mBeginScene.first->Undetour();
    if (pDevice->TestCooperativeLevel() == 0) {
      for (const auto& ext : mBeginSceneExts)
        if (ext) ext(pDevice);
    }

    auto _ret = mBeginScene.second(pDevice);
    mBeginScene.first->Detour();
    return _ret;
  }
  HRESULT __stdcall hkEndScene(D3D9::LPDIRECT3DDEVICE9 pDevice) {
    mEndScene.first->Undetour();
    if (pDevice->TestCooperativeLevel() == 0) {
      for (const auto& ext : mEndSceneExts)
        if (ext) ext(pDevice);
    }

    auto _ret = mEndScene.second(pDevice);
    mEndScene.first->Detour();
    return _ret;
  }
  HRESULT __stdcall hkReset(D3D9::LPDIRECT3DDEVICE9 pDevice, D3D9::D3DPRESENT_PARAMETERS* pPresentationParameters) {
    mReset.first->Undetour();

    for (const auto& ext : mBefResetExts)
      if (ext) ext(pDevice, pPresentationParameters);

    auto _ret = mReset.second(pDevice, pPresentationParameters);

    for (const auto& ext : mAftResetExts)
      if (ext) ext(pDevice, pPresentationParameters);

    mReset.first->Detour();
    return _ret;
  }
  HRESULT __stdcall hkBeginStateBlock(D3D9::LPDIRECT3DDEVICE9 pDevice) {
    mBeginScene.first.reset();
    mEndScene.first.reset();
    mReset.first.reset();
    mBeginStateBlock.first.reset();
    auto _ret = mBeginStateBlock.second(pDevice);

    auto* vtDevice = *(std::uintptr_t**)pDevice;
    mBeginScene =
        std::make_pair(MemoryEditor::Get().Detour(vtDevice[41], reinterpret_cast<std::uintptr_t>(&hkBeginScene)),
                       reinterpret_cast<D3D9::BeginScene_t>(vtDevice[41]));
    mEndScene = std::make_pair(MemoryEditor::Get().Detour(vtDevice[42], reinterpret_cast<std::uintptr_t>(&hkEndScene)),
                               reinterpret_cast<D3D9::EndScene_t>(vtDevice[42]));
    mReset    = std::make_pair(MemoryEditor::Get().Detour(vtDevice[16], reinterpret_cast<std::uintptr_t>(&hkReset)),
                            reinterpret_cast<D3D9::Reset_t>(vtDevice[16]));
    mBeginStateBlock =
        std::make_pair(MemoryEditor::Get().Detour(vtDevice[60], reinterpret_cast<std::uintptr_t>(&hkBeginStateBlock)),
                       reinterpret_cast<D3D9::BeginStateBlock_t>(vtDevice[60]));
    return _ret;
  }
#pragma endregion
#pragma region Exported
  bool __stdcall AddExtension(D3D9Extension type, void* pExtension) {
#pragma ExportedFunction
    std::scoped_lock<std::mutex> _l(mMutex);

    switch (type) {
      case D3D9Extension::BeginScene:
        mBeginSceneExts.push_back(reinterpret_cast<D3D9::BeginScene_t>(pExtension));
        break;
      case D3D9Extension::EndScene:
        mEndSceneExts.push_back(reinterpret_cast<D3D9::EndScene_t>(pExtension));
        break;
      case D3D9Extension::BeforeReset:
        mBefResetExts.push_back(reinterpret_cast<D3D9::Reset_t>(pExtension));
        break;
      case D3D9Extension::AfterReset:
        mAftResetExts.push_back(reinterpret_cast<D3D9::Reset_t>(pExtension));
        break;
      default:
        return false;
    }
    return true;
  }
#pragma endregion

  bool Init(D3D9::LPDIRECT3DDEVICE9* ppDevice) {
    if (!GetModuleHandle(TEXT("d3d9.dll"))) return false;

    while (!*ppDevice) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto mDevice = *ppDevice;

    D3D9::D3DDEVICE_CREATION_PARAMETERS cParams;
    mDevice->GetCreationParameters(&cParams);

    auto* vtDevice = **(std::uintptr_t***)ppDevice;
    mBeginScene =
        std::make_pair(MemoryEditor::Get().Detour(vtDevice[41], reinterpret_cast<std::uintptr_t>(&hkBeginScene)),
                       reinterpret_cast<D3D9::BeginScene_t>(vtDevice[41]));
    mEndScene = std::make_pair(MemoryEditor::Get().Detour(vtDevice[42], reinterpret_cast<std::uintptr_t>(&hkEndScene)),
                               reinterpret_cast<D3D9::EndScene_t>(vtDevice[42]));
    mReset    = std::make_pair(MemoryEditor::Get().Detour(vtDevice[16], reinterpret_cast<std::uintptr_t>(&hkReset)),
                            reinterpret_cast<D3D9::Reset_t>(vtDevice[16]));
    mBeginStateBlock =
        std::make_pair(MemoryEditor::Get().Detour(vtDevice[60], reinterpret_cast<std::uintptr_t>(&hkBeginStateBlock)),
                       reinterpret_cast<D3D9::BeginStateBlock_t>(vtDevice[60]));

    WndProcExtender::Init(cParams.hFocusWindow);
    return true;
  }
  bool Init(HWND& hWindow) {
    auto hLib = GetModuleHandle(TEXT("d3d9.dll"));
    if (!hLib) return false;

    D3D9::D3DPRESENT_PARAMETERS params = {0};
    {
      params.SwapEffect    = 1;
      params.hDeviceWindow = hWindow;
      params.Windowed      = FALSE;
    }

    D3D9::LPDIRECT3DDEVICE9 mDevice;
    D3D9::LPDIRECT3D9       pD3D =
        reinterpret_cast<D3D9::LPDIRECT3D9(__stdcall*)(UINT)>(GetProcAddress(hLib, "Direct3DCreate9"))(32);
    if (FAILED(pD3D->CreateDevice(0, 1, hWindow, 32, &params, &mDevice))) {
      params.Windowed = TRUE;
      if (FAILED(pD3D->CreateDevice(0, 1, hWindow, 32, &params, &mDevice))) {
        pD3D->Release();
        return false;
      }
    }

    auto* vtDevice = *(std::uintptr_t**)mDevice;
    mBeginScene =
        std::make_pair(MemoryEditor::Get().Detour(vtDevice[41], reinterpret_cast<std::uintptr_t>(&hkBeginScene)),
                       reinterpret_cast<D3D9::BeginScene_t>(vtDevice[41]));
    mEndScene = std::make_pair(MemoryEditor::Get().Detour(vtDevice[42], reinterpret_cast<std::uintptr_t>(&hkEndScene)),
                               reinterpret_cast<D3D9::EndScene_t>(vtDevice[42]));
    mReset    = std::make_pair(MemoryEditor::Get().Detour(vtDevice[16], reinterpret_cast<std::uintptr_t>(&hkReset)),
                            reinterpret_cast<D3D9::Reset_t>(vtDevice[16]));
    mBeginStateBlock =
        std::make_pair(MemoryEditor::Get().Detour(vtDevice[60], reinterpret_cast<std::uintptr_t>(&hkBeginStateBlock)),
                       reinterpret_cast<D3D9::BeginStateBlock_t>(vtDevice[60]));

    mDevice->Release();
    mDevice = nullptr;
    pD3D->Release();
    pD3D = nullptr;

    WndProcExtender::Init(hWindow);
    return true;
  }
  bool Init(const wchar_t* const szWindowTitle) {
    if (auto hWindow = FindWindowW(0, szWindowTitle)) return Init(hWindow);
    return false;
  }
}  // namespace MirrorHookInternals::D3D9Extender
