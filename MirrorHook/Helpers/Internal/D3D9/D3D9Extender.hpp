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
#include "Helpers/Internal/D3D9/D3D9Types.h"
#include "Helpers/Internal/WndProc/WndProcExtender.hpp"
#include "Helpers/Internal/imgui/imgui.h"
#include "Helpers/Internal/imgui/dx9/imgui_impl_dx9.h"
#include "Helpers/Internal/imgui/win32/imgui_impl_win32.h"

#include "inc/Definitions.hpp"
using MirrorHook::D3D9::D3D9Extension;

namespace MirrorHookInternals {
   namespace D3D9Extender {
      LPDIRECT3DDEVICE9 d3dDevice                 = nullptr;
      HWND              windowHandle              = nullptr;

      auto              vBeginSceneExtensions     = std::vector<BeginScene_t>();
      auto              vEndSceneExtensions       = std::vector<EndScene_t>();
      auto              vBeforeResetExtensions    = std::vector<Reset_t>();
      auto              vAfterResetExtensions     = std::vector<Reset_t>();

      bool              useImGui                  = true;
      bool              isImGuiReady              = false;
      unsigned int      infoOverlayFrame          = 301;
      unsigned int      infoOverlayFrame_MaxFrame = 300;

      bool              isExtenderReady           = false;

   #pragma region function hooks
      std::unique_ptr<VTableHook> d3dDeviceHook       = nullptr;
      BeginScene_t                origBeginScene      = nullptr;
      EndScene_t                  origEndScene        = nullptr;
      Reset_t                     origReset           = nullptr;
      BeginStateBlock_t           origBeginStateBlock = nullptr;

      HRESULT __stdcall hkBeginScene(LPDIRECT3DDEVICE9 pDevice) {
         if (pDevice->TestCooperativeLevel() == D3D_OK) {
            if (!vBeginSceneExtensions.empty()) {
               for (BeginScene_t beginSceneExtension : vBeginSceneExtensions) {
                  if (beginSceneExtension)
                     beginSceneExtension(pDevice);
               }
            }

            if (useImGui && !isImGuiReady) {
               ImGui_ImplDX9::Init(d3dDevice);
               ImGui_ImplWin32_Init(windowHandle);

               isImGuiReady = true;
            }
            if (useImGui && isImGuiReady) {
               ImGui_ImplDX9::NewFrame();
               ImGui_ImplWin32_NewFrame();
               ImGui::NewFrame();
            }
         }
         return origBeginScene(pDevice);
      }
      HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
         if (pDevice->TestCooperativeLevel() == D3D_OK) {
            if (!vEndSceneExtensions.empty()) {
               for (EndScene_t endSceneExtension : vEndSceneExtensions) {
                  if (endSceneExtension)
                     endSceneExtension(pDevice);
               }
            }
            if (useImGui && isImGuiReady) {
               if (infoOverlayFrame_MaxFrame == -1
                   || infoOverlayFrame < infoOverlayFrame_MaxFrame) {
                  ImGui::SetNextWindowPos(ImVec2(10.0f, 40.0f), ImGuiCond_Once);
                  if (ImGui::Begin("##MirrorHook", nullptr,
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize)) {

                     ImGui::Text(MirrorHookVersionInfo);
                     ImGui::Text("https://github.com/berkayylmao/MirrorHook");
                     ImGui::Text("by berkayylmao");
                     ImGui::Separator();

                     ImGui::Text("D3D9 Extender Information");
                     ImGui::Indent(5.0f);
                     {
                        ImGui::Text("BeginScene  | EndScene extensions   : %d | %d", vBeginSceneExtensions.size(), vEndSceneExtensions.size());
                        ImGui::Text("BeforeReset | AfterReset extensions : %d | %d", vBeforeResetExtensions.size(), vAfterResetExtensions.size());
                     }
                     ImGui::Unindent(5.0f);

                     if (infoOverlayFrame_MaxFrame != -1) {
                        ImGui::Separator();
                        ImGui::Text("I will disappear in... %04u.", infoOverlayFrame_MaxFrame - infoOverlayFrame);

                        infoOverlayFrame++;
                        if (infoOverlayFrame >= infoOverlayFrame_MaxFrame) {
                           ImGui::End();
                           ImGui::Render();
                           ImGui_ImplDX9::RenderDrawData(ImGui::GetDrawData());
                           useImGui = false;
                           return origEndScene(pDevice);
                        }
                     }
                  }
                  ImGui::End();
                  ImGui::Render();
                  ImGui_ImplDX9::RenderDrawData(ImGui::GetDrawData());
               }
            }
         }
         return origEndScene(pDevice);
      }
      HRESULT __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
         if (!vBeforeResetExtensions.empty()) {
            for (Reset_t beforeResetExtension : vBeforeResetExtensions) {
               if (beforeResetExtension)
                  beforeResetExtension(pDevice, pPresentationParameters);
            }
         }

         if (isImGuiReady)
            ImGui_ImplDX9::InvalidateDeviceObjects();

         auto retOrigReset = origReset(pDevice, pPresentationParameters);

         if (!vAfterResetExtensions.empty()) {
            for (Reset_t afterResetExtension : vAfterResetExtensions) {
               if (afterResetExtension)
                  afterResetExtension(pDevice, pPresentationParameters);
            }
         }

         if (isImGuiReady)
            ImGui_ImplDX9::CreateDeviceObjects();

         return retOrigReset;
      }
      HRESULT __stdcall hkBeginStateBlock(LPDIRECT3DDEVICE9 pDevice) {
         d3dDeviceHook->UnhookAll();

         auto retBeginStateBlock = origBeginStateBlock(pDevice);

         origReset                = d3dDeviceHook->Hook(16, hkReset);
         origBeginScene           = d3dDeviceHook->Hook(41, hkBeginScene);
         origEndScene             = d3dDeviceHook->Hook(42, hkEndScene);
         origBeginStateBlock      = d3dDeviceHook->Hook(60, hkBeginStateBlock);
         return retBeginStateBlock;
      }
   #pragma endregion

   #pragma region exported helpers
      LPDIRECT3DDEVICE9 __stdcall GetD3D9Device() {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         return d3dDevice;
      }
      HWND __stdcall GetWindowHandle() {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         return windowHandle;
      }
      bool __stdcall AddExtension(D3D9Extension extensionType, LPVOID extensionAddress) {
      #pragma ExportedFunction
         switch (extensionType) {
            case D3D9Extension::BeginScene:
               vBeginSceneExtensions.push_back(reinterpret_cast<BeginScene_t>(extensionAddress));
               break;
            case D3D9Extension::EndScene:
               vEndSceneExtensions.push_back(reinterpret_cast<EndScene_t>(extensionAddress));
               break;
            case D3D9Extension::BeforeReset:
               vBeforeResetExtensions.push_back(reinterpret_cast<Reset_t>(extensionAddress));
               break;
            case D3D9Extension::AfterReset:
               vAfterResetExtensions.push_back(reinterpret_cast<Reset_t>(extensionAddress));
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
         infoOverlayFrame = 0;
         windowHandle     = *pWindowHandle;

         throw std::exception("Not implemented.");

         d3dDeviceHook       = std::make_unique<VTableHook>((PDWORD_PTR*)d3dDevice);
         origReset           = d3dDeviceHook->Hook(16, hkReset);
         origBeginScene      = d3dDeviceHook->Hook(41, hkBeginScene);
         origEndScene        = d3dDeviceHook->Hook(42, hkEndScene);
         origBeginStateBlock = d3dDeviceHook->Hook(60, hkBeginStateBlock);

         WndProcExtender::Init(pWindowHandle);
         isExtenderReady = true;
         return true;
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