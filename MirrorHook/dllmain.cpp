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

#include "stdafx.h"
#include "Helpers/Memory/Memory.h"
#include "Helpers/Memory/Memory64.hpp"
#include "Helpers/Memory/VTableHook.hpp"
#include "Helpers/Memory/VTableHook64.hpp"

#include "Helpers/Internal/DI8/DI8Types.h"
#include "Helpers/Internal/D3D9/D3D9Types.h"
#include "Helpers/Internal/D3D11/D3D11Types.h"

#include "imgui.h"
#include "dx9/imgui_impl_dx9.h"
#include "dx11/imgui_impl_dx11.h"

#include "inc/Definitions.hpp"
using MirrorHook::DI8::DI8Device;
using MirrorHook::DI8::DI8Extension;
using MirrorHook::D3D9::D3D9Extension;
using MirrorHook::D3D11::D3D11Extension;

#include <mutex>
#include <memory>
using std::unique_ptr;
using std::make_unique;
#include <map>
using std::map;
#include <vector>
using std::vector;

namespace MirrorHookInternals {
   namespace DI8Extender {
      DWORD                    dinput8Address                    = NULL;

      LPDIRECTINPUT8A          di8Instance                       = nullptr;
      LPDIRECTINPUTDEVICE8A    device_Keyboard                   = nullptr;
      LPDIRECTINPUTDEVICE8A    device_Mouse                      = nullptr;

      map<DI8Device, vector<GetDeviceState_t>> mGetDeviceStateExtensions;

      bool                     isExtenderReady                   = false;

   #pragma region function hooks
      GetDeviceState_t         origGetDeviceState_Keyboard       = nullptr;
      GetDeviceState_t         origGetDeviceState_Mouse          = nullptr;

      HRESULT WINAPI hkGetDeviceState_Keyboard(HINSTANCE hInstance, DWORD cbData, LPVOID lpvData) {
         HRESULT retOrigGetDeviceState = origGetDeviceState_Keyboard(hInstance, cbData, lpvData);

         if (!mGetDeviceStateExtensions[DI8Device::Keyboard].empty()) {
            for (GetDeviceState_t keyboardExtension : mGetDeviceStateExtensions[DI8Device::Keyboard]) {
               if (keyboardExtension)
                  keyboardExtension(hInstance, cbData, lpvData);
            }
         }
         return retOrigGetDeviceState;
      }
      HRESULT WINAPI hkGetDeviceState_Mouse(HINSTANCE hInstance, DWORD cbData, LPVOID lpvData) {
         HRESULT retOrigGetDeviceState = origGetDeviceState_Mouse(hInstance, cbData, lpvData);

         if (!mGetDeviceStateExtensions[DI8Device::Mouse].empty()) {
            for (GetDeviceState_t mouseExtension : mGetDeviceStateExtensions[DI8Device::Mouse]) {
               if (mouseExtension)
                  mouseExtension(hInstance, cbData, lpvData);
            }
         }
         return retOrigGetDeviceState;
      }
   #pragma endregion

   #pragma region exported helpers
      HRESULT WINAPI AddExtension(DI8Device deviceType, DI8Extension extensionType, LPVOID extensionAddress) {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return FALSE;

         switch (extensionType) {
            case DI8Extension::GetDeviceState:
            {
               switch (deviceType) {
                  case DI8Device::Keyboard:
                  case DI8Device::Mouse:
                     mGetDeviceStateExtensions[deviceType].push_back(reinterpret_cast<GetDeviceState_t>(extensionAddress));
                     break;
                  default:
                     return FALSE;
               }
               break;
            }
            default:
               return FALSE;
         }
         return TRUE;
      }
      LPDIRECTINPUT8A WINAPI GetDirectInput8A() {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         return di8Instance;
      }
      LPDIRECTINPUTDEVICE8A WINAPI GetDirectInputDevice8A(DI8Device deviceType) {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         switch (deviceType) {
            case DI8Device::Keyboard:
               return device_Keyboard;
            case DI8Device::Mouse:
               return device_Mouse;
         }
         return nullptr;
      }
      bool WINAPI IsReady() {
      #pragma ExportedFunction
         return isExtenderReady;
      }
   #pragma endregion

      BOOL CALLBACK enumCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID) {
         BYTE deviceType = LOBYTE(lpddi->dwDevType);
         if (deviceType == DI8DEVTYPE_KEYBOARD || deviceType == DI8DEVTYPE_MOUSE) {
            DWORD* inputTable;

            if (deviceType == DI8DEVTYPE_KEYBOARD) {
               di8Instance->CreateDevice(lpddi->guidInstance, &device_Keyboard, NULL);
               inputTable = *(PDWORD*)device_Keyboard;
            } else {
               di8Instance->CreateDevice(lpddi->guidInstance, &device_Mouse, NULL);
               inputTable = *(PDWORD*)device_Mouse;
            }

            Memory::openMemoryAccess(inputTable[9], 4);

            if (deviceType == DI8DEVTYPE_KEYBOARD) {
               origGetDeviceState_Keyboard = (GetDeviceState_t)(DWORD)inputTable[9];
               inputTable[9]               = (DWORD)hkGetDeviceState_Keyboard;
            } else {
               origGetDeviceState_Mouse    = (GetDeviceState_t)(DWORD)inputTable[9];
               inputTable[9]               = (DWORD)hkGetDeviceState_Mouse;
            }

            Memory::restoreMemoryAccess();
         }

         if (device_Keyboard && device_Mouse) {
            isExtenderReady = true;
            return DIENUM_STOP;
         }
         return DIENUM_CONTINUE;
      }
      void Init() {
         mGetDeviceStateExtensions[DI8Device::Keyboard] = vector<GetDeviceState_t>();
         mGetDeviceStateExtensions[DI8Device::Mouse]    = vector<GetDeviceState_t>();

         DWORD pDI8 = NULL;
         while (!pDI8) {
            pDI8 = *(DWORD*)dinput8Address;
            Sleep(100);
         }
         di8Instance = (LPDIRECTINPUT8A)pDI8;
         di8Instance->EnumDevices(DI8DEVCLASS_ALL, &enumCallback, NULL, DIEDFL_ATTACHEDONLY);
      }
   }
   namespace D3D9Extender {
      DWORD                  d3dDeviceAddress      = NULL;

      LPDIRECT3DDEVICE9      d3dDevice             = nullptr;
      HWND                   d3dWindow             = nullptr;

      TestCooperativeLevel_t testCooperativeLevelExtension = nullptr;
      vector<BeginScene_t>   vBeginSceneExtensions         = vector<BeginScene_t>();
      vector<EndScene_t>     vEndSceneExtensions           = vector<EndScene_t>();
      vector<Reset_t>        vBeforeResetExtensions        = vector<Reset_t>();
      vector<Reset_t>        vAfterResetExtensions         = vector<Reset_t>();

      bool                   useImGui                  = true;
      bool                   isImGuiReady              = false;
      unsigned int           infoOverlayFrame          = 301;
      unsigned int           infoOverlayFrame_MaxFrame = 300;

      bool                   isExtenderReady = false;

   #pragma region function hooks
      unique_ptr<VTableHook> d3dDeviceHook            = nullptr;
      TestCooperativeLevel_t origTestCooperativeLevel = nullptr;
      BeginScene_t           origBeginScene           = nullptr;
      EndScene_t             origEndScene             = nullptr;
      Reset_t                origReset                = nullptr;
      BeginStateBlock_t      origBeginStateBlock      = nullptr;

      HRESULT WINAPI hkTestCooperativeLevel(LPDIRECT3DDEVICE9 pDevice) {
         HRESULT hr = origTestCooperativeLevel(pDevice);
         if (testCooperativeLevelExtension) {
            if (HRESULT exRet = testCooperativeLevelExtension(pDevice)) {
               return exRet;
            }
         }
         return hr;
      }
      HRESULT WINAPI hkBeginScene(LPDIRECT3DDEVICE9 pDevice) {
         if (pDevice->TestCooperativeLevel() == D3D_OK) {
            if (!vBeginSceneExtensions.empty()) {
               for (BeginScene_t beginSceneExtension : vBeginSceneExtensions) {
                  if (beginSceneExtension)
                     beginSceneExtension(pDevice);
               }
            }

            if (ImGui::IsKeyPressed(VK_F9, false)) {
               infoOverlayFrame_MaxFrame = -1;
               useImGui = !useImGui;
            }
            ImGui::GetIO().KeysDown[VK_F9] = GetKeyState(VK_F9) & 0x8000;

            if (useImGui && !isImGuiReady) {
               ImGui_ImplDX9_Init(d3dWindow, d3dDevice);
               ImGuiIO& io = ImGui::GetIO();
               io.IniFilename = NULL;

               isImGuiReady = true;
            }
            ImGui_ImplDX9_NewFrame();
         }
         return origBeginScene(pDevice);
      }
      HRESULT WINAPI hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
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

                     ImGui::Text("MirrorHook v1.0");
                     ImGui::Text("https://github.com/berkayylmao/MirrorHook");
                     ImGui::Text("by berkay(2578)");
                     ImGui::Separator();

                     ImGui::Text("D3D9 Extender         : %s", D3D9Extender::isExtenderReady ? "Ready" : "Not ready"); // redundant much?
                     ImGui::Text("DirectInput8 Extender : %s", DI8Extender::isExtenderReady ? "Ready" : "Not ready");
                     ImGui::Separator();

                     ImGui::Text("D3D9 Extender Information");
                     ImGui::Indent(5.0f);
                     {
                        ImGui::Text("BeginScene  | EndScene extensions   : %d | %d", vBeginSceneExtensions.size(), vEndSceneExtensions.size());
                        ImGui::Text("BeforeReset | AfterReset extensions : %d | %d", vBeforeResetExtensions.size(), vAfterResetExtensions.size());
                     }
                     ImGui::Unindent(5.0f);
                     ImGui::Separator();

                     ImGui::Text("DirectInput8 Extender Information");
                     ImGui::Indent(5.0f);
                     {
                        ImGui::Text("Keyboard | Mouse");
                        ImGui::Indent(2.5f);
                        {
                           ImGui::Text("GetDeviceState extensions : %d | %d",
                                       DI8Extender::mGetDeviceStateExtensions[DI8Device::Keyboard].size(),
                                       DI8Extender::mGetDeviceStateExtensions[DI8Device::Mouse].size());
                        }
                        ImGui::Unindent(2.5f);
                     }
                     ImGui::Unindent(5.0f);
                     ImGui::Separator();
                     ImGui::Text("Press F9 to toggle me.");

                     if (infoOverlayFrame_MaxFrame != -1) {
                        ImGui::Text("I will disappear in... %04u.", infoOverlayFrame_MaxFrame - infoOverlayFrame);

                        infoOverlayFrame++;
                        if (infoOverlayFrame >= infoOverlayFrame_MaxFrame) {
                           ImGui::End();
                           ImGui::Render();
                           useImGui = false;
                           isImGuiReady = false;
                           return origEndScene(pDevice);
                        }
                     }
                  }
                  ImGui::End();
                  ImGui::Render();
               }
            }
         }
         return origEndScene(pDevice);
      }
      HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
         if (!vBeforeResetExtensions.empty()) {
            for (Reset_t beforeResetExtension : vBeforeResetExtensions) {
               if (beforeResetExtension)
                  beforeResetExtension(pDevice, pPresentationParameters);
            }
         }

         if (isImGuiReady)
            ImGui_ImplDX9_InvalidateDeviceObjects();

         auto retOrigReset = origReset(pDevice, pPresentationParameters);

         if (!vAfterResetExtensions.empty()) {
            for (Reset_t afterResetExtension : vAfterResetExtensions) {
               if (afterResetExtension)
                  afterResetExtension(pDevice, pPresentationParameters);
            }
         }

         if (isImGuiReady)
            ImGui_ImplDX9_CreateDeviceObjects();

         return retOrigReset;
      }
      HRESULT WINAPI hkBeginStateBlock(LPDIRECT3DDEVICE9 pDevice) {
         d3dDeviceHook->UnhookAll();

         auto retBeginStateBlock = origBeginStateBlock(pDevice);

         origTestCooperativeLevel = d3dDeviceHook->Hook(3, hkTestCooperativeLevel);
         origReset                = d3dDeviceHook->Hook(16, hkReset);
         origBeginScene           = d3dDeviceHook->Hook(41, hkBeginScene);
         origEndScene             = d3dDeviceHook->Hook(42, hkEndScene);
         origBeginStateBlock      = d3dDeviceHook->Hook(60, hkBeginStateBlock);
         return retBeginStateBlock;
      }
   #pragma endregion

   #pragma region exported helpers
      HRESULT WINAPI AddExtension(D3D9Extension extensionType, LPVOID extensionAddress) {
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
               return FALSE;
         }
         return TRUE;
      }
      HRESULT WINAPI SetTestCooperativeLevelExtension(LPVOID extensionAddress) {
      #pragma ExportedFunction
         testCooperativeLevelExtension = reinterpret_cast<TestCooperativeLevel_t>(extensionAddress);
         return TRUE;
      }
      LPDIRECT3DDEVICE9 WINAPI GetD3D9Device() {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         return d3dDevice;
      }
      HWND WINAPI GetWindowHandle() {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         return d3dWindow;
      }
      bool WINAPI IsReady() {
      #pragma ExportedFunction
         return isExtenderReady;
      }
   #pragma endregion

      void Init() {
         infoOverlayFrame = 0;
         DWORD pD3DDevice = NULL;
         while (!pD3DDevice) {
            pD3DDevice = *(DWORD*)d3dDeviceAddress;
            Sleep(100);
         }
         d3dDevice = (LPDIRECT3DDEVICE9)pD3DDevice;

         D3DDEVICE_CREATION_PARAMETERS cParams;
         d3dDevice->GetCreationParameters(&cParams);
         d3dWindow = cParams.hFocusWindow;

         d3dDeviceHook            = make_unique<VTableHook>((PDWORD*)d3dDevice);
         origTestCooperativeLevel = d3dDeviceHook->Hook(3, hkTestCooperativeLevel);
         origReset                = d3dDeviceHook->Hook(16, hkReset);
         origBeginScene           = d3dDeviceHook->Hook(41, hkBeginScene);
         origEndScene             = d3dDeviceHook->Hook(42, hkEndScene);
         origBeginStateBlock      = d3dDeviceHook->Hook(60, hkBeginStateBlock);

         isExtenderReady = true;
      }
   }
   namespace D3D11Extender {
      ID3D11Device*          pD3DDevice                = nullptr;
      ID3D11DeviceContext*   pD3DContext               = nullptr;
      IDXGISwapChain*        pSwapChain                = nullptr;

      auto                   vPresentExtensions        = vector<D3D11Types::Present_t>();

      bool                   useImGui                  = true;
      bool                   isImGuiReady              = false;
      unsigned int           infoOverlayFrame          = 301;
      unsigned int           infoOverlayFrame_MaxFrame = 300;

      bool                   isExtenderReady           = false;
      std::once_flag         isExtenderReadyLock;

   #pragma region function hooks
      unique_ptr<VTableHook64> d3dDeviceHook             = nullptr;
      D3D11Types::Present_t  origPresent               = nullptr;

      HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
         if (!vPresentExtensions.empty()) {
            for (auto& presentExtension : vPresentExtensions) {
               if (presentExtension)
                  presentExtension(pSwapChain, SyncInterval, Flags);
            }
         }

         if (useImGui && !isImGuiReady) {
            std::call_once(isExtenderReadyLock, [&]() {
               pSwapChain->GetDevice(__uuidof(pD3DDevice), reinterpret_cast<void**>(&pD3DDevice));
               pD3DDevice->GetImmediateContext(&pD3DContext);

               ImGui_ImplDX11_Init(pD3DDevice, pD3DContext);
                           }
            );
         }
         if (useImGui && isImGuiReady) {
            ImGui::GetIO().KeysDown[VK_F9] = GetKeyState(VK_F9) & 0x8000;
            if (ImGui::IsKeyPressed(VK_F9, false)) {
               infoOverlayFrame_MaxFrame = -1;
               useImGui = !useImGui;
            }

            if (infoOverlayFrame_MaxFrame == -1
                || infoOverlayFrame < infoOverlayFrame_MaxFrame) {
               ImGui::SetNextWindowPos(ImVec2(10.0f, 40.0f), ImGuiCond_Once);
               if (ImGui::Begin("##MirrorHook", nullptr,
                                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize)) {

                  ImGui::Text("MirrorHook v1.1");
                  ImGui::Text("https://github.com/berkayylmao/MirrorHook");
                  ImGui::Text("by berkayylmao");
                  ImGui::Separator();

                  ImGui::Text("D3D11 Extender Information");
                  ImGui::Indent(5.0f);
                  {
                     ImGui::Text("Present extensions: %d", vPresentExtensions.size());
                  }
                  ImGui::Unindent(5.0f);
                  ImGui::Separator();
                  ImGui::Text("Press F9 to toggle me.");

                  if (infoOverlayFrame_MaxFrame != -1) {
                     ImGui::Text("I will disappear in... %04u.", infoOverlayFrame_MaxFrame - infoOverlayFrame);

                     infoOverlayFrame++;
                     if (infoOverlayFrame >= infoOverlayFrame_MaxFrame) {
                        ImGui::End();
                        ImGui::Render();
                        useImGui = false;
                        isImGuiReady = false;
                        return origPresent(pSwapChain, SyncInterval, Flags);
                     }
                  }
               }
               ImGui::End();
               ImGui::Render();
            }
         }
         return origPresent(pSwapChain, SyncInterval, Flags);
      }
   #pragma endregion

   #pragma region exported helpers
      HRESULT __stdcall AddExtension(D3D11Extension extensionType, LPVOID extensionAddress) {
      #pragma ExportedFunction
         switch (extensionType) {
            case D3D11Extension::Present:
               vPresentExtensions.push_back(reinterpret_cast<D3D11Types::Present_t>(extensionAddress));
               break;
            default:
               return FALSE;
         }
         return TRUE;
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

         return pD3DContext;
      }
      bool __stdcall IsReady() {
      #pragma ExportedFunction
         return isExtenderReady;
      }
   #pragma endregion

      void Init() {
         infoOverlayFrame = 0;
         d3dDeviceHook    = make_unique<VTableHook64>((PDWORD64*)pSwapChain);
         origPresent      = d3dDeviceHook->Hook(8, hkPresent);

         isExtenderReady = true;
      }
   }

   bool isInit = false;
   DWORD __stdcall Init(LPVOID) {
      if (DI8Extender::dinput8Address)
         DI8Extender::Init();
      if (D3D9Extender::d3dDeviceAddress)
         D3D9Extender::Init();
      if (D3D11Extender::pD3DDevice)
         D3D11Extender::Init();

      isInit = true;
      return TRUE;
   }

#pragma region exported helpers
   bool __stdcall PrepareFor(MirrorHook::Game gameType, const wchar_t* windowTitleName) {
   #pragma ExportedFunction
      if (!isInit && !DI8Extender::isExtenderReady && !D3D9Extender::isExtenderReady) {
         Memory::Init();
         switch (gameType) {
            case MirrorHook::Game::MostWanted:
            {
               DI8Extender::dinput8Address    = Memory::makeAbsolute(0x582D14);
               D3D9Extender::d3dDeviceAddress = Memory::makeAbsolute(0x582BDC);
            }
            break;
            case MirrorHook::Game::Carbon:
            {
               DI8Extender::dinput8Address    = Memory::makeAbsolute(0x71F5CC);
               D3D9Extender::d3dDeviceAddress = Memory::makeAbsolute(0x6B0ABC);
            }
            case MirrorHook::Game::UniversalD3D11:
            {
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
                  sd.OutputWindow = FindWindowW(0, windowTitleName);
                  sd.SampleDesc.Count = 1;
                  sd.Windowed = ((GetWindowLongPtr(sd.OutputWindow, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
                  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
                  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
                  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

                  sd.BufferDesc.Width = 1;
                  sd.BufferDesc.Height = 1;
                  sd.BufferDesc.RefreshRate.Numerator = 0;
                  sd.BufferDesc.RefreshRate.Denominator = 1;
               }

               HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &sd, &D3D11Extender::pSwapChain, &D3D11Extender::pD3DDevice, &obtainedLevel, &D3D11Extender::pD3DContext);
               if (FAILED(hr)) {
                  printf("Failed to create device and swapchain.");
                  return false;
               }
            }
            break;
         }
         CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Init, 0, 0, 0);
         return true;
      } else return false;
   }
   bool WINAPI IsReady() {
   #pragma ExportedFunction
      return isInit;
   }
   bool WINAPI IsShowingInfoOverlay() {
   #pragma ExportedFunction
      return (D3D9Extender::infoOverlayFrame < D3D9Extender::infoOverlayFrame_MaxFrame) ||
         (D3D11Extender::infoOverlayFrame < D3D11Extender::infoOverlayFrame_MaxFrame);
   }
#pragma endregion

   BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
      if (ul_reason_for_call == DLL_PROCESS_ATTACH)
         DisableThreadLibraryCalls(hModule);
      return TRUE;
   }
}