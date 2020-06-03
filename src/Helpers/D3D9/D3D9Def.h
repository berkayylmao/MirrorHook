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
#pragma warning(push, 0)
#include <WinDef.h>
#pragma warning(pop)

namespace MirrorHookInternals::D3D9 {
#pragma region Copyright(C) Microsoft Corporation.All Rights Reserved.
  struct D3DPRESENT_PARAMETERS {
    UINT BackBufferWidth;
    UINT BackBufferHeight;
    INT  BackBufferFormat;
    UINT BackBufferCount;

    INT   MultiSampleType;
    DWORD MultiSampleQuality;

    INT   SwapEffect;
    HWND  hDeviceWindow;
    BOOL  Windowed;
    BOOL  EnableAutoDepthStencil;
    INT   AutoDepthStencilFormat;
    DWORD Flags;

    /* FullScreen_RefreshRateInHz must be zero for Windowed mode */
    UINT FullScreen_RefreshRateInHz;
    UINT PresentationInterval;
  };
  struct D3DDEVICE_CREATION_PARAMETERS {
    UINT  AdapterOrdinal;
    INT   DeviceType;
    HWND  hFocusWindow;
    DWORD BehaviorFlags;
  };

  struct IDirect3DDevice9 {
    void* unused1[1];
    virtual ULONG __stdcall AddRef() noexcept                 = 0;
    virtual ULONG __stdcall Release() noexcept                = 0;
    virtual HRESULT __stdcall TestCooperativeLevel() noexcept = 0;
    void* unused2[5];
    virtual HRESULT __stdcall GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameneters) = 0;
    void* unused3[6];
    virtual HRESULT __stdcall Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) = 0;
    void* unused4[25];
    virtual HRESULT __stdcall BeginScene() = 0;
    virtual HRESULT __stdcall EndScene()   = 0;
    void* unused5[18];
    virtual HRESULT __stdcall BeginStateBlock() = 0;
  };
  struct IDirect3D9 {
    void* unused1[1];
    virtual ULONG __stdcall AddRef() noexcept  = 0;
    virtual ULONG __stdcall Release() noexcept = 0;
    void* unused2[13];
    virtual HRESULT __stdcall CreateDevice(UINT Adapter, INT DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
                                           void*              pPresentationParameters,
                                           IDirect3DDevice9** ppReturnedDeviceInterface) noexcept = 0;
  };

  typedef IDirect3DDevice9* LPDIRECT3DDEVICE9;
  typedef IDirect3D9*       LPDIRECT3D9;
#pragma endregion

  typedef HRESULT(__stdcall* Reset_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
  typedef HRESULT(__stdcall* BeginScene_t)(LPDIRECT3DDEVICE9 pDevice);
  typedef HRESULT(__stdcall* EndScene_t)(LPDIRECT3DDEVICE9 pDevice);
  typedef HRESULT(__stdcall* BeginStateBlock_t)(LPDIRECT3DDEVICE9 pDevice);
}  // namespace MirrorHookInternals::D3D9
