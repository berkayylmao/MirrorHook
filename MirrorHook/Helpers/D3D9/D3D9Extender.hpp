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
#include "Helpers/D3D9/D3D9Types.h"
#include "Helpers/Memory/DetourHook.hpp"
#include "Helpers/Memory/VTableHook.hpp"
#include "Helpers/WndProc/WndProcExtender.hpp"
#include "Helpers/Dear ImGui/imgui.h"
#include "Helpers/Dear ImGui/Implementations/imgui_impl_dx9.h"
#include "Helpers/Dear ImGui/Implementations/imgui_impl_win32.h"

using MirrorHook::D3D9::D3D9Extension;

namespace MirrorHookInternals {
  namespace D3D9Extender {
    std::mutex                           d3d9Mutex;
    LPDIRECT3DDEVICE9                    d3dDevice    = nullptr;
    HWND                                 windowHandle = nullptr;
    std::vector<D3D9Types::BeginScene_t> vBeginSceneExtensions;
    std::vector<D3D9Types::EndScene_t>   vEndSceneExtensions;
    std::vector<D3D9Types::Reset_t>      vBeforeResetExtensions;
    std::vector<D3D9Types::Reset_t>      vAfterResetExtensions;

    bool     isExtenderReady           = false;
    bool     isUsingVTableHook         = false;
    bool     useImGui                  = true;
    bool     isImGuiReady              = false;
    uint32_t infoOverlayFrame          = 0;
    uint32_t infoOverlayFrame_MaxFrame = 300;

#pragma region function hooks
    std::unique_ptr<Helpers::DetourHook> d3dDeviceDetourHook = nullptr;
    std::unique_ptr<Helpers::VTableHook> d3dDeviceVTableHook = nullptr;
    D3D9Types::BeginScene_t              origBeginScene      = nullptr;
    D3D9Types::EndScene_t                origEndScene        = nullptr;
    D3D9Types::Reset_t                   origReset           = nullptr;
    D3D9Types::BeginStateBlock_t         origBeginStateBlock = nullptr;

    HRESULT __stdcall hkBeginScene(LPDIRECT3DDEVICE9 pDevice) {
      if (pDevice->TestCooperativeLevel() == D3D_OK) {
        if (!vBeginSceneExtensions.empty()) {
          for (D3D9Types::BeginScene_t beginSceneExtension : vBeginSceneExtensions) {
            if (beginSceneExtension) beginSceneExtension(pDevice);
          }
        }

        if (useImGui && !isImGuiReady) {
          if (d3dDevice == nullptr) d3dDevice = pDevice;

          ImGui::CreateContext();
          ImGui_ImplDX9::Init(pDevice);
          ImGui_ImplWin32_Init(windowHandle);

          isImGuiReady = true;
        }
        if (useImGui && isImGuiReady) {
          ImGui_ImplDX9::NewFrame();
          ImGui_ImplWin32_NewFrame();
          ImGui::NewFrame();
        }
      }

      HRESULT ret = D3D_OK;
      if (isUsingVTableHook) {
        ret = origBeginScene(pDevice);
      } else {
        auto* pDI = d3dDeviceDetourHook->GetInfoOf(origBeginScene);
        pDI->Unhook();
        auto ret = origBeginScene(pDevice);
        pDI->Hook();
      }
      return ret;
    }
    HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
      if (pDevice->TestCooperativeLevel() == D3D_OK) {
        if (!vEndSceneExtensions.empty()) {
          for (D3D9Types::EndScene_t endSceneExtension : vEndSceneExtensions) {
            if (endSceneExtension) endSceneExtension(pDevice);
          }
        }
        if (useImGui && isImGuiReady && infoOverlayFrame < infoOverlayFrame_MaxFrame) {
          ImGui::SetNextWindowPos(ImVec2(10.0f, 40.0f), ImGuiCond_Once);
          if (ImGui::Begin("##MirrorHook", nullptr,
                           ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                               ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(MirrorHookVersionInfo);
            ImGui::Text("https://github.com/berkayylmao/MirrorHook");
            ImGui::Text("by berkayylmao");
            ImGui::Separator();

            ImGui::Text("D3D9 Extender Information");
            ImGui::Indent(5.0f);
            {
              ImGui::Text("BeginScene  | EndScene extensions   : %d | %d",
                          vBeginSceneExtensions.size(), vEndSceneExtensions.size());
              ImGui::Text("BeforeReset | AfterReset extensions : %d | %d",
                          vBeforeResetExtensions.size(), vAfterResetExtensions.size());
            }
            ImGui::Unindent(5.0f);

            ImGui::Separator();
            ImGui::Text("I will disappear in... %04u.",
                        infoOverlayFrame_MaxFrame - infoOverlayFrame);

            infoOverlayFrame++;
            useImGui = (infoOverlayFrame < infoOverlayFrame_MaxFrame);
          }
          ImGui::End();
          ImGui::Render();
          ImGui_ImplDX9::RenderDrawData(ImGui::GetDrawData());
        }
      }

      HRESULT ret = D3D_OK;
      if (isUsingVTableHook) {
        ret = origEndScene(pDevice);
      } else {
        auto* pDI = d3dDeviceDetourHook->GetInfoOf(origEndScene);
        pDI->Unhook();
        ret = origEndScene(pDevice);
        pDI->Hook();
      }
      return ret;
    }
    HRESULT __stdcall hkReset(LPDIRECT3DDEVICE9      pDevice,
                              D3DPRESENT_PARAMETERS* pPresentationParameters) {
      if (!vBeforeResetExtensions.empty()) {
        for (D3D9Types::Reset_t beforeResetExtension : vBeforeResetExtensions) {
          if (beforeResetExtension) beforeResetExtension(pDevice, pPresentationParameters);
        }
      }

      if (isImGuiReady) ImGui_ImplDX9::InvalidateDeviceObjects();

      HRESULT ret = D3D_OK;
      if (isUsingVTableHook) {
        ret = origReset(pDevice, pPresentationParameters);
      } else {
        auto* pDI = d3dDeviceDetourHook->GetInfoOf(origReset);
        pDI->Unhook();
        auto ret = origReset(pDevice, pPresentationParameters);
        pDI->Hook();
      }

      if (!vAfterResetExtensions.empty()) {
        for (D3D9Types::Reset_t afterResetExtension : vAfterResetExtensions) {
          if (afterResetExtension) afterResetExtension(pDevice, pPresentationParameters);
        }
      }

      if (isImGuiReady) ImGui_ImplDX9::CreateDeviceObjects();

      return ret;
    }
    HRESULT __stdcall hkBeginStateBlock(LPDIRECT3DDEVICE9 pDevice) {
      HRESULT ret = D3D_OK;
      if (isUsingVTableHook) {
        d3dDeviceVTableHook->UnhookAll();
        ret = origBeginStateBlock(pDevice);

        origReset           = d3dDeviceVTableHook->Hook(16, hkReset);
        origBeginScene      = d3dDeviceVTableHook->Hook(41, hkBeginScene);
        origEndScene        = d3dDeviceVTableHook->Hook(42, hkEndScene);
        origBeginStateBlock = d3dDeviceVTableHook->Hook(60, hkBeginStateBlock);
      } else {
        d3dDeviceDetourHook->UnhookAll();
        ret = origBeginStateBlock(pDevice);

        DWORD_PTR* vtDevice = *(PDWORD_PTR*)pDevice;
        origReset           = d3dDeviceDetourHook->Hook(vtDevice[16], hkReset);
        origBeginScene      = d3dDeviceDetourHook->Hook(vtDevice[41], hkBeginScene);
        origEndScene        = d3dDeviceDetourHook->Hook(vtDevice[42], hkEndScene);
        origBeginStateBlock = d3dDeviceDetourHook->Hook(vtDevice[60], hkBeginStateBlock);
      }

      return ret;
    }
#pragma endregion

#pragma region exported helpers
    LPDIRECT3DDEVICE9 __stdcall GetD3D9Device() {
#pragma ExportedFunction
      if (!isExtenderReady) return nullptr;

      return d3dDevice;
    }
    bool __stdcall AddExtension(D3D9Extension extensionType, LPVOID extensionAddress) {
#pragma ExportedFunction
      std::scoped_lock _lock(d3d9Mutex);
      switch (extensionType) {
        case D3D9Extension::BeginScene:
          vBeginSceneExtensions.push_back(
              reinterpret_cast<D3D9Types::BeginScene_t>(extensionAddress));
          break;
        case D3D9Extension::EndScene:
          vEndSceneExtensions.push_back(reinterpret_cast<D3D9Types::EndScene_t>(extensionAddress));
          break;
        case D3D9Extension::BeforeReset:
          vBeforeResetExtensions.push_back(reinterpret_cast<D3D9Types::Reset_t>(extensionAddress));
          break;
        case D3D9Extension::AfterReset:
          vAfterResetExtensions.push_back(reinterpret_cast<D3D9Types::Reset_t>(extensionAddress));
          break;
        default:
          return false;
      }
      return true;
    }
    bool __stdcall IsReady() {
#pragma ExportedFunction
      return isExtenderReady;
    }
#pragma endregion

    bool Init(LPDIRECT3DDEVICE9* ppDevice) {
      while (!*ppDevice) Sleep(100);
      d3dDevice = *ppDevice;

      D3DDEVICE_CREATION_PARAMETERS cParams;
      d3dDevice->GetCreationParameters(&cParams);
      windowHandle = cParams.hFocusWindow;

      d3dDeviceVTableHook = std::make_unique<Helpers::VTableHook>((PDWORD_PTR*)d3dDevice);
      origReset           = d3dDeviceVTableHook->Hook(16, hkReset);
      origBeginScene      = d3dDeviceVTableHook->Hook(41, hkBeginScene);
      origEndScene        = d3dDeviceVTableHook->Hook(42, hkEndScene);
      origBeginStateBlock = d3dDeviceVTableHook->Hook(60, hkBeginStateBlock);

      WndProcExtender::Init(&windowHandle);
      isUsingVTableHook = true;
      isExtenderReady   = true;
      return true;
    }
    bool Init(HWND* pWindowHandle) {
      windowHandle = *pWindowHandle;
      if (windowHandle == nullptr) return false;

      D3DPRESENT_PARAMETERS params = {0};
      {
        params.SwapEffect    = D3DSWAPEFFECT_DISCARD;
        params.hDeviceWindow = windowHandle;
        params.Windowed      = FALSE;
      }

      LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
      if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &d3dDevice))) {
        params.Windowed = TRUE;
        if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &d3dDevice))) {
          pD3D->Release();
          return false;
        }
      }

      DWORD_PTR* vtDevice = *(PDWORD_PTR*)d3dDevice;
      d3dDeviceDetourHook = std::make_unique<Helpers::DetourHook>();
      origReset           = d3dDeviceDetourHook->Hook(vtDevice[16], hkReset);
      origBeginScene      = d3dDeviceDetourHook->Hook(vtDevice[41], hkBeginScene);
      origEndScene        = d3dDeviceDetourHook->Hook(vtDevice[42], hkEndScene);
      origBeginStateBlock = d3dDeviceDetourHook->Hook(vtDevice[60], hkBeginStateBlock);

      d3dDevice->Release();
      d3dDevice = nullptr;
      pD3D->Release();
      pD3D = nullptr;

      WndProcExtender::Init(pWindowHandle);
      isExtenderReady = true;
      return true;
    }
    bool Init(const TCHAR* const windowTitleName) {
      if (windowHandle = FindWindow(0, windowTitleName)) return Init(&windowHandle);
      return false;
    }
  }  // namespace D3D9Extender
}  // namespace MirrorHookInternals