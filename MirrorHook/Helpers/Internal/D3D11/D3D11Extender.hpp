/*
   MIT License

   Copyright (c) 2019 Berkay Yigit <berkay2578@gmail.com>
       Copyright holder detail: Nickname(s) used by the copyright holder: 'berkay2578', 'berkayylmao'.

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
#include "Helpers/Internal/Memory/Memory.hpp"
#include "Helpers/Internal/Memory/VTableHook.hpp"
#include "Helpers/Internal/D3D11/D3D11Types.h"
#include "Helpers/Internal/WndProc/WndProcExtender.hpp"
#include "Helpers/Internal/imgui/imgui.h"
#include "Helpers/Internal/imgui/dx11/imgui_impl_dx11.h"
#include "Helpers/Internal/imgui/win32/imgui_impl_win32.h"

#include "inc/Definitions.hpp"
using MirrorHook::D3D11::D3D11Extension;

namespace MirrorHookInternals {
   namespace D3D11Extender {
      HWND                    windowHandle              = nullptr;
      ID3D11Device*           pD3DDevice                = nullptr;
      ID3D11DeviceContext*    pD3DDeviceContext         = nullptr;

      auto                    vPresentExtensions        = std::vector<D3D11Types::Present_t>();

      bool                    useImGui                  = true;
      bool                    isImGuiReady              = false;
      unsigned int            infoOverlayFrame          = 301;
      unsigned int            infoOverlayFrame_MaxFrame = 300;

      bool                    isExtenderReady           = false;
      static std::once_flag   isExtenderReadyLock;

   #pragma region function hooks
      std::unique_ptr<VTableHook> dxgiSwapChainHook          = nullptr;
      D3D11Types::Present_t         origPresent              = nullptr;

      HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain_Inner, UINT SyncInterval, UINT Flags) {
         if (useImGui && !isImGuiReady) {
            std::call_once(isExtenderReadyLock, [&]() {
               pSwapChain_Inner->GetDevice(__uuidof(pD3DDevice), reinterpret_cast<void**>(&pD3DDevice));
               pD3DDevice->GetImmediateContext(&pD3DDeviceContext);

               ImGui::CreateContext();
               ImGui_ImplDX11::Init(pD3DDevice, pD3DDeviceContext);
               ImGui_ImplWin32_Init(windowHandle);
               isImGuiReady = true;
                           }
            );
         }

         if (!vPresentExtensions.empty()) {
            for (auto& presentExtension : vPresentExtensions) {
               if (presentExtension)
                  presentExtension(pSwapChain_Inner, SyncInterval, Flags);
            }
         }

         if (useImGui && isImGuiReady) {
            if (infoOverlayFrame_MaxFrame == -1
                || infoOverlayFrame < infoOverlayFrame_MaxFrame) {
               ImGui_ImplDX11::NewFrame();
               ImGui_ImplWin32_NewFrame();
               ImGui::NewFrame();

               ImGui::SetNextWindowPos(ImVec2(10.0f, 40.0f), ImGuiCond_Once);
               if (ImGui::Begin("##MirrorHook", nullptr,
                                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize)) {

                  ImGui::Text(MirrorHookVersionInfo);
                  ImGui::Text("https://github.com/berkayylmao/MirrorHook");
                  ImGui::Text("by berkayylmao");
                  ImGui::Separator();

                  ImGui::Text("D3D11 Extender Information");
                  ImGui::Indent(5.0f);
                  {
                     ImGui::Text("Present extensions: %d", vPresentExtensions.size());
                  }
                  ImGui::Unindent(5.0f);

                  if (infoOverlayFrame_MaxFrame != -1) {
                     ImGui::Separator();
                     ImGui::Text("I will disappear in... %04u.", infoOverlayFrame_MaxFrame - infoOverlayFrame);

                     infoOverlayFrame++;
                     if (infoOverlayFrame >= infoOverlayFrame_MaxFrame) {
                        ImGui::End();
                        ImGui::Render();
                        ImGui_ImplDX11::RenderDrawData(ImGui::GetDrawData());
                        useImGui = false;
                        return origPresent(pSwapChain_Inner, SyncInterval, Flags);
                     }
                  }
               }
               ImGui::End();
               ImGui::Render();
               ImGui_ImplDX11::RenderDrawData(ImGui::GetDrawData());
            }
         }
         return origPresent(pSwapChain_Inner, SyncInterval, Flags);
      }
   #pragma endregion

   #pragma region exported helpers
      HWND __stdcall GetWindowHandle() {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         return windowHandle;
      }
      ID3D11Device* __stdcall GetD3D11Device() {
      #pragma ExportedFunction
         if (isExtenderReady)
            return nullptr;

         return pD3DDevice;
      }
      ID3D11DeviceContext* __stdcall GetD3D11DeviceContext() {
      #pragma ExportedFunction
         if (isExtenderReady)
            return nullptr;

         return pD3DDeviceContext;
      }
      bool __stdcall AddExtension(D3D11Extension extensionType, LPVOID extensionAddress) {
      #pragma ExportedFunction
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

      DWORD64 getRealSwapChainAddress(DWORD_PTR vtblSwapChain) {
      #ifdef _WIN64
         MEMORY_BASIC_INFORMATION64 mbi ={ 0 };
      #else
         MEMORY_BASIC_INFORMATION32 mbi ={ 0 };
      #endif
         DWORD_PTR addrVtbl;
         for (DWORD_PTR memptr = 0x10000; memptr < 0x7FFFFFFEFFFF; memptr = mbi.BaseAddress + mbi.RegionSize) {
            if (!VirtualQuery(reinterpret_cast<LPCVOID>(memptr), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi), sizeof(mbi)))
               continue;
            if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS || mbi.Protect & PAGE_GUARD)
               continue;

            DWORD_PTR len = mbi.BaseAddress + mbi.RegionSize;
            for (DWORD_PTR current = mbi.BaseAddress; current < len; current++) {
               __try {
                  if (*(DWORD_PTR*)current == vtblSwapChain)
                     return current;
               } __except (EXCEPTION_EXECUTE_HANDLER) {
                  continue;
               }
            }
         }

         return NULL;
      }
      bool Init(HWND* pWindowHandle) {
         infoOverlayFrame = 0;
         windowHandle     = *pWindowHandle;

         // Get D3D11 device and swap chain
         D3D_FEATURE_LEVEL levels[] ={ D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
         D3D_FEATURE_LEVEL obtainedLevel;
         DXGI_SWAP_CHAIN_DESC sd;
         {
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = 1;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            sd.OutputWindow = windowHandle;
            sd.SampleDesc.Count = 1;
            sd.Windowed = ((GetWindowLongPtrA(windowHandle, GWL_STYLE) & WS_POPUP) != 0) ? false : true;;
            sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

            sd.BufferDesc.Width = 1;
            sd.BufferDesc.Height = 1;
            sd.BufferDesc.RefreshRate.Numerator = 0;
            sd.BufferDesc.RefreshRate.Denominator = 1;
         }

         IDXGISwapChain* pFakeSwapChain;
         if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &sd, &pFakeSwapChain, &pD3DDevice, &obtainedLevel, &pD3DDeviceContext)))
            return false;

         DWORD_PTR vtblSwapChain = *(DWORD_PTR*)pFakeSwapChain;
         pD3DDevice->Release();
         pD3DDeviceContext->Release();
         pFakeSwapChain->Release();
         pFakeSwapChain = nullptr;

         dxgiSwapChainHook = std::make_unique<VTableHook>((PDWORD_PTR*)getRealSwapChainAddress(vtblSwapChain));
         origPresent       = dxgiSwapChainHook->Hook(8, hkPresent);

         WndProcExtender::Init(pWindowHandle);
         isExtenderReady = true;
         return origPresent != nullptr;
      }
      bool Init(const TCHAR* const windowTitleName) {
      #ifdef UNICODE
         if (windowHandle = FindWindowW(0, windowTitleName))
            return Init(&windowHandle);
      #else
         if (windowHandle = FindWindowA(0, windowTitleName))
            return Init(&windowHandle);
      #endif
         return false;
      }
   }
}