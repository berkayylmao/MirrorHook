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
#include "Helpers/D3D11/D3D11Types.h"
#include "Helpers/Memory/DetourHook.hpp"
#include "Helpers/WndProc/WndProcExtender.hpp"
#include "Helpers/Dear ImGui/imgui.h"
#include "Helpers/Dear ImGui/Implementations/imgui_impl_dx11.h"
#include "Helpers/Dear ImGui/Implementations/imgui_impl_win32.h"

using MirrorHook::D3D11::D3D11Extension;

namespace MirrorHookInternals {
  namespace D3D11Extender {
    std::mutex                         d3d11Mutex;
    ID3D11Device*                      pD3DDevice        = nullptr;
    ID3D11DeviceContext*               pD3DDeviceContext = nullptr;
    ID3D11RenderTargetView*            pRenderTargetView = nullptr;
    HWND                               windowHandle      = nullptr;
    std::vector<D3D11Types::Present_t> vPresentExtensions;

    bool     isExtenderReady           = false;
    bool     useImGui                  = true;
    uint32_t infoOverlayFrame          = 0;
    uint32_t infoOverlayFrame_MaxFrame = 500;

#pragma region function hooks
    std::unique_ptr<Helpers::DetourHook> dxgiSwapChainDetourHook = nullptr;
    D3D11Types::Present_t                origPresent             = nullptr;

    HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
      if (!isExtenderReady) {
        pSwapChain->GetDevice(__uuidof(pD3DDevice), reinterpret_cast<void**>(&pD3DDevice));
        pD3DDevice->GetImmediateContext(&pD3DDeviceContext);

        if (useImGui) {
          ImGui::CreateContext();
          ImGui_ImplDX11::Init(pD3DDevice, pD3DDeviceContext);
          ImGui_ImplWin32_Init(windowHandle);
        }
        isExtenderReady = true;
      }

      if (!vPresentExtensions.empty()) {
        for (auto& presentExtension : vPresentExtensions) {
          if (presentExtension) presentExtension(pSwapChain, SyncInterval, Flags);
        }
      }

      if (useImGui && infoOverlayFrame < infoOverlayFrame_MaxFrame) {
        ImGui_ImplDX11::NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10.0f, 40.0f), ImGuiCond_Once);
        if (ImGui::Begin("##MirrorHook", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_AlwaysAutoResize)) {
          ImGui::Text(MirrorHookVersionInfo);
          ImGui::Text("https://github.com/berkayylmao/MirrorHook");
          ImGui::Text("by berkayylmao");
          ImGui::Separator();

          ImGui::Text("D3D11 Extender Information");
          ImGui::Indent(5.0f);
          { ImGui::Text("Present extensions: %d", vPresentExtensions.size()); }
          ImGui::Unindent(5.0f);

          ImGui::Separator();
          ImGui::Text("I will disappear in... %04u.", infoOverlayFrame_MaxFrame - infoOverlayFrame);

          infoOverlayFrame++;
          useImGui = (infoOverlayFrame < infoOverlayFrame_MaxFrame);
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplDX11::RenderDrawData(ImGui::GetDrawData());

        if (!useImGui) {
          ImGui_ImplWin32_Shutdown();
          ImGui_ImplDX11::Shutdown();
          ImGui::DestroyContext();
        }
      }

      auto* pDI = dxgiSwapChainDetourHook->GetInfoOf(origPresent);
      pDI->Unhook();
      auto ret = origPresent(pSwapChain, SyncInterval, Flags);
      pDI->Hook();
      return ret;
    }
#pragma endregion

#pragma region exported helpers
    ID3D11Device* __stdcall GetD3D11Device() {
#pragma ExportedFunction
      if (!isExtenderReady) return nullptr;

      return pD3DDevice;
    }
    ID3D11DeviceContext* __stdcall GetD3D11DeviceContext() {
#pragma ExportedFunction
      if (!isExtenderReady) return nullptr;

      return pD3DDeviceContext;
    }
    bool __stdcall AddExtension(D3D11Extension extensionType, LPVOID extensionAddress) {
#pragma ExportedFunction
      std::scoped_lock _lock(d3d11Mutex);
      switch (extensionType) {
        case D3D11Extension::Present:
          vPresentExtensions.push_back(reinterpret_cast<D3D11Types::Present_t>(extensionAddress));
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

    bool Init(HWND* pWindowHandle) {
      windowHandle = *pWindowHandle;
      if (windowHandle == nullptr) return false;

      D3D_FEATURE_LEVEL    levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
      D3D_FEATURE_LEVEL    obtainedLevel;
      DXGI_SWAP_CHAIN_DESC sd;
      {
        sd.BufferCount       = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow      = windowHandle;
        sd.SampleDesc.Count  = 1;
        sd.Windowed          = TRUE;
        sd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags             = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
      }

      IDXGISwapChain* pFakeSwapChain;
      if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                               levels, sizeof(levels) / sizeof(D3D_FEATURE_LEVEL),
                                               D3D11_SDK_VERSION, &sd, &pFakeSwapChain, &pD3DDevice,
                                               &obtainedLevel, &pD3DDeviceContext)))
        return false;

      DWORD_PTR* vtDevice     = *(PDWORD_PTR*)pFakeSwapChain;
      dxgiSwapChainDetourHook = std::make_unique<Helpers::DetourHook>();
      origPresent             = dxgiSwapChainDetourHook->Hook(vtDevice[8], hkPresent);

      pFakeSwapChain->Release();
      pD3DDevice->Release();
      pD3DDeviceContext->Release();

      pFakeSwapChain    = nullptr;
      pD3DDevice        = nullptr;
      pD3DDeviceContext = nullptr;

      WndProcExtender::Init(pWindowHandle);
      return true;
    }
    bool Init(const TCHAR* const windowTitleName) {
      if (windowHandle = FindWindow(0, windowTitleName)) return Init(&windowHandle);
      return false;
    }
  }  // namespace D3D11Extender
}  // namespace MirrorHookInternals