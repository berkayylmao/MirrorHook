/*
   The MIT License (MIT)

   Copyright (c) 2020 Berkay Yigit <berkaytgy@gmail.com>
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
#pragma warning(push, 0)
#include <d3d9.h>
#pragma warning(pop)

namespace D3D9Types {
   typedef HRESULT(__stdcall* QueryInterface_t)(LPDIRECT3DDEVICE9 pDevice, REFIID riid, void** ppvObj);
   typedef ULONG(__stdcall* AddRef_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef ULONG(__stdcall* Release_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* TestCooperativeLevel_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef UINT(__stdcall* GetAvailableTextureMem_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* EvictManagedResources_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* GetDirect3D_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3D9** ppD3D9);
   typedef HRESULT(__stdcall* GetDeviceCaps_t)(LPDIRECT3DDEVICE9 pDevice, D3DCAPS9* pCaps);
   typedef HRESULT(__stdcall* GetDisplayMode_t)(LPDIRECT3DDEVICE9 pDevice, UINT iSwapChain, D3DDISPLAYMODE* pMode);
   typedef HRESULT(__stdcall* GetCreationParameters_t)(LPDIRECT3DDEVICE9 pDevice, D3DDEVICE_CREATION_PARAMETERS *pParameters);
   typedef HRESULT(__stdcall* SetCursorProperties_t)(LPDIRECT3DDEVICE9 pDevice, UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap);
   typedef void(__stdcall* SetCursorPosition_t)(LPDIRECT3DDEVICE9 pDevice, int X, int Y, DWORD Flags);
   typedef BOOL(__stdcall* ShowCursor_t)(LPDIRECT3DDEVICE9 pDevice, BOOL bShow);
   typedef HRESULT(__stdcall* CreateAdditionalSwapChain_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain);
   typedef HRESULT(__stdcall* GetSwapChain_t)(LPDIRECT3DDEVICE9 pDevice, UINT iSwapChain, IDirect3DSwapChain9** pSwapChain);
   typedef UINT(__stdcall* GetNumberOfSwapChains_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* Reset_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
   typedef HRESULT(__stdcall* Present_t)(LPDIRECT3DDEVICE9 pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion);
   typedef HRESULT(__stdcall* GetBackBuffer_t)(LPDIRECT3DDEVICE9 pDevice, UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
   typedef HRESULT(__stdcall* GetRasterStatus_t)(LPDIRECT3DDEVICE9 pDevice, UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus);
   typedef HRESULT(__stdcall* SetDialogBoxMode_t)(LPDIRECT3DDEVICE9 pDevice, BOOL bEnableDialogs);
   typedef void(__stdcall* SetGammaRamp_t)(LPDIRECT3DDEVICE9 pDevice, UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP* pRamp);
   typedef void(__stdcall* GetGammaRamp_t)(LPDIRECT3DDEVICE9 pDevice, UINT iSwapChain, D3DGAMMARAMP* pRamp);
   typedef HRESULT(__stdcall* CreateTexture_t)(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* CreateVolumeTexture_t)(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* CreateCubeTexture_t)(LPDIRECT3DDEVICE9 pDevice, UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* CreateVertexBuffer_t)(LPDIRECT3DDEVICE9 pDevice, UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* CreateIndexBuffer_t)(LPDIRECT3DDEVICE9 pDevice, UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* CreateRenderTarget_t)(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* CreateDepthStencilSurface_t)(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* UpdateSurface_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, const POINT* pDestPoint);
   typedef HRESULT(__stdcall* UpdateTexture_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture);
   typedef HRESULT(__stdcall* GetRenderTargetData_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface);
   typedef HRESULT(__stdcall* GetFrontBufferData_t)(LPDIRECT3DDEVICE9 pDevice, UINT iSwapChain, IDirect3DSurface9* pDestSurface);
   typedef HRESULT(__stdcall* StretchRect_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestSurface, const RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
   typedef HRESULT(__stdcall* ColorFill_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9* pSurface, const RECT* pRect, D3DCOLOR color);
   typedef HRESULT(__stdcall* CreateOffscreenPlainSurface_t)(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
   typedef HRESULT(__stdcall* SetRenderTarget_t)(LPDIRECT3DDEVICE9 pDevice, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
   typedef HRESULT(__stdcall* GetRenderTarget_t)(LPDIRECT3DDEVICE9 pDevice, DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget);
   typedef HRESULT(__stdcall* SetDepthStencilSurface_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9* pNewZStencil);
   typedef HRESULT(__stdcall* GetDepthStencilSurface_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DSurface9** ppZStencilSurface);
   typedef HRESULT(__stdcall* BeginScene_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* EndScene_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* Clear_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
   typedef HRESULT(__stdcall* SetTransform_t)(LPDIRECT3DDEVICE9 pDevice, D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix);
   typedef HRESULT(__stdcall* GetTransform_t)(LPDIRECT3DDEVICE9 pDevice, D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
   typedef HRESULT(__stdcall* MultiplyTransform_t)(LPDIRECT3DDEVICE9 pDevice, D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix);
   typedef HRESULT(__stdcall* SetViewport_t)(LPDIRECT3DDEVICE9 pDevice, const D3DVIEWPORT9* pViewport);
   typedef HRESULT(__stdcall* GetViewport_t)(LPDIRECT3DDEVICE9 pDevice, D3DVIEWPORT9* pViewport);
   typedef HRESULT(__stdcall* SetMaterial_t)(LPDIRECT3DDEVICE9 pDevice, const D3DMATERIAL9* pMaterial);
   typedef HRESULT(__stdcall* GetMaterial_t)(LPDIRECT3DDEVICE9 pDevice, D3DMATERIAL9* pMaterial);
   typedef HRESULT(__stdcall* SetLight_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Index, const D3DLIGHT9* pLight);
   typedef HRESULT(__stdcall* GetLight_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Index, D3DLIGHT9* pLight);
   typedef HRESULT(__stdcall* LightEnable_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Index, BOOL Enable);
   typedef HRESULT(__stdcall* GetLightEnable_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Index, BOOL* pEnable);
   typedef HRESULT(__stdcall* SetClipPlane_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Index, const float* pPlane);
   typedef HRESULT(__stdcall* GetClipPlane_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Index, float* pPlane);
   typedef HRESULT(__stdcall* SetRenderState_t)(LPDIRECT3DDEVICE9 pDevice, D3DRENDERSTATETYPE State, DWORD Value);
   typedef HRESULT(__stdcall* GetRenderState_t)(LPDIRECT3DDEVICE9 pDevice, D3DRENDERSTATETYPE State, DWORD* pValue);
   typedef HRESULT(__stdcall* CreateStateBlock_t)(LPDIRECT3DDEVICE9 pDevice, D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB);
   typedef HRESULT(__stdcall* BeginStateBlock_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* EndStateBlock_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DStateBlock9** ppSB);
   typedef HRESULT(__stdcall* SetClipStatus_t)(LPDIRECT3DDEVICE9 pDevice, const D3DCLIPSTATUS9* pClipStatus);
   typedef HRESULT(__stdcall* GetClipStatus_t)(LPDIRECT3DDEVICE9 pDevice, D3DCLIPSTATUS9* pClipStatus);
   typedef HRESULT(__stdcall* GetTexture_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Stage, IDirect3DBaseTexture9** ppTexture);
   typedef HRESULT(__stdcall* SetTexture_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Stage, IDirect3DBaseTexture9* pTexture);
   typedef HRESULT(__stdcall* GetTextureStageState_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
   typedef HRESULT(__stdcall* SetTextureStageState_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
   typedef HRESULT(__stdcall* GetSamplerState_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
   typedef HRESULT(__stdcall* SetSamplerState_t)(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
   typedef HRESULT(__stdcall* ValidateDevice_t)(LPDIRECT3DDEVICE9 pDevice, DWORD* pNumPasses);
   typedef HRESULT(__stdcall* SetPaletteEntries_t)(LPDIRECT3DDEVICE9 pDevice, UINT PaletteNumber, const PALETTEENTRY* pEntries);
   typedef HRESULT(__stdcall* GetPaletteEntries_t)(LPDIRECT3DDEVICE9 pDevice, UINT PaletteNumber, PALETTEENTRY* pEntries);
   typedef HRESULT(__stdcall* SetCurrentTexturePalette_t)(LPDIRECT3DDEVICE9 pDevice, UINT PaletteNumber);
   typedef HRESULT(__stdcall* GetCurrentTexturePalette_t)(LPDIRECT3DDEVICE9 pDevice, UINT *PaletteNumber);
   typedef HRESULT(__stdcall* SetScissorRect_t)(LPDIRECT3DDEVICE9 pDevice, const RECT* pRect);
   typedef HRESULT(__stdcall* GetScissorRect_t)(LPDIRECT3DDEVICE9 pDevice, RECT* pRect);
   typedef HRESULT(__stdcall* SetSoftwareVertexProcessing_t)(LPDIRECT3DDEVICE9 pDevice, BOOL bSoftware);
   typedef BOOL(__stdcall* GetSoftwareVertexProcessing_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* SetNPatchMode_t)(LPDIRECT3DDEVICE9 pDevice, float nSegments);
   typedef float(__stdcall* GetNPatchMode_t)(LPDIRECT3DDEVICE9 pDevice);
   typedef HRESULT(__stdcall* DrawPrimitive_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
   typedef HRESULT(__stdcall* DrawIndexedPrimitive_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
   typedef HRESULT(__stdcall* DrawPrimitiveUP_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
   typedef HRESULT(__stdcall* DrawIndexedPrimitiveUP_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
   typedef HRESULT(__stdcall* ProcessVertices_t)(LPDIRECT3DDEVICE9 pDevice, UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags);
   typedef HRESULT(__stdcall* CreateVertexDeclaration_t)(LPDIRECT3DDEVICE9 pDevice, const D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl);
   typedef HRESULT(__stdcall* SetVertexDeclaration_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pDecl);
   typedef HRESULT(__stdcall* GetVertexDeclaration_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9** ppDecl);
   typedef HRESULT(__stdcall* SetFVF_t)(LPDIRECT3DDEVICE9 pDevice, DWORD FVF);
   typedef HRESULT(__stdcall* GetFVF_t)(LPDIRECT3DDEVICE9 pDevice, DWORD* pFVF);
   typedef HRESULT(__stdcall* CreateVertexShader_t)(LPDIRECT3DDEVICE9 pDevice, const DWORD* pFunction, IDirect3DVertexShader9** ppShader);
   typedef HRESULT(__stdcall* SetVertexShader_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9* pShader);
   typedef HRESULT(__stdcall* GetVertexShader_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9** ppShader);
   typedef HRESULT(__stdcall* SetVertexShaderConstantF_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float* pconstantData, UINT Vector4fCount);
   typedef HRESULT(__stdcall* GetVertexShaderConstantF_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, float* pconstantData, UINT Vector4fCount);
   typedef HRESULT(__stdcall* SetVertexShaderConstantI_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const int* pconstantData, UINT Vector4iCount);
   typedef HRESULT(__stdcall* GetVertexShaderConstantI_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, int* pconstantData, UINT Vector4iCount);
   typedef HRESULT(__stdcall* SetVertexShaderConstantB_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const BOOL* pconstantData, UINT BoolCount);
   typedef HRESULT(__stdcall* GetVertexShaderConstantB_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, BOOL* pconstantData, UINT BoolCount);
   typedef HRESULT(__stdcall* SetStreamSource_t)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
   typedef HRESULT(__stdcall* GetStreamSource_t)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride);
   typedef HRESULT(__stdcall* SetStreamSourceFreq_t)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, UINT Divider);
   typedef HRESULT(__stdcall* GetStreamSourceFreq_t)(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, UINT* Divider);
   typedef HRESULT(__stdcall* SetIndices_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DIndexBuffer9* pIndexData);
   typedef HRESULT(__stdcall* GetIndices_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DIndexBuffer9** ppIndexData);
   typedef HRESULT(__stdcall* CreatePixelShader_t)(LPDIRECT3DDEVICE9 pDevice, const DWORD* pFunction, IDirect3DPixelShader9** ppShader);
   typedef HRESULT(__stdcall* SetPixelShader_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9* pShader);
   typedef HRESULT(__stdcall* GetPixelShader_t)(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9** ppShader);
   typedef HRESULT(__stdcall* SetPixelShaderConstantF_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float* pconstantData, UINT Vector4fCount);
   typedef HRESULT(__stdcall* GetPixelShaderConstantF_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, float* pconstantData, UINT Vector4fCount);
   typedef HRESULT(__stdcall* SetPixelShaderConstantI_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const int* pconstantData, UINT Vector4iCount);
   typedef HRESULT(__stdcall* GetPixelShaderConstantI_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, int* pconstantData, UINT Vector4iCount);
   typedef HRESULT(__stdcall* SetPixelShaderConstantB_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const BOOL* pconstantData, UINT BoolCount);
   typedef HRESULT(__stdcall* GetPixelShaderConstantB_t)(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, BOOL* pconstantData, UINT BoolCount);
   typedef HRESULT(__stdcall* DrawRectPatch_t)(LPDIRECT3DDEVICE9 pDevice, UINT Handle, const float* pNumSegs, const D3DRECTPATCH_INFO* pRectPatchInfo);
   typedef HRESULT(__stdcall* DrawTriPatch_t)(LPDIRECT3DDEVICE9 pDevice, UINT Handle, const float* pNumSegs, const D3DTRIPATCH_INFO* pTriPatchInfo);
   typedef HRESULT(__stdcall* DeletePatch_t)(LPDIRECT3DDEVICE9 pDevice, UINT Handle);
   typedef HRESULT(__stdcall* CreateQuery_t)(LPDIRECT3DDEVICE9 pDevice, D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);
}