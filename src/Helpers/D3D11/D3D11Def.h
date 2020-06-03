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

namespace MirrorHookInternals::D3D11 {
#pragma region Copyright(C) Microsoft Corporation.All Rights Reserved.
  struct DXGI_RATIONAL {
    UINT Numerator;
    UINT Denominator;
  };
  struct DXGI_MODE_DESC {
    UINT          Width;
    UINT          Height;
    DXGI_RATIONAL RefreshRate;
    INT           Format;
    INT           ScanlineOrdering;
    INT           Scaling;
  };
  struct DXGI_SAMPLE_DESC {
    UINT Count;
    UINT Quality;
  };
  struct DXGI_SWAP_CHAIN_DESC {
    DXGI_MODE_DESC   BufferDesc;
    DXGI_SAMPLE_DESC SampleDesc;
    INT              BufferUsage;
    UINT             BufferCount;
    HWND             OutputWindow;
    BOOL             Windowed;
    INT              SwapEffect;
    UINT             Flags;
  };
  struct IUnknown {
    void* unused1[2];
    virtual ULONG __stdcall Release() = 0;
  };
  typedef IUnknown IDXGISwapChain;
  typedef IUnknown ID3D11Device;
  typedef IUnknown ID3D11DeviceContext;
#pragma endregion

  typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
}  // namespace MirrorHookInternals::D3D11