# MirrorHook
A wrapper for using the existing D3D9 and D3D11+ implementations in software. In addition, after setting up MirrorHook, it will expose the window's `WndProc` via `MirrorHook::WndProc::*`.

After MirrorHook is injected, there are 2 ways to utilize it:
1. Prepare MirrorHook manually by calling one of the `MirrorHook::PrepareFor(\*)` functions.
2. Wait for MirrorHook to detect the underlying DX implementation (takes maximum of 5 seconds), call `MirrorHook::GetInstalledFramework()` to find the implementation and then use the according extender namespace (`MirrorHook::D3D9Extender` & `MirrorHook::D3D11Extender`).


## Example implementation:
```
// Example:
#include "path/to/MirrorHook/inc/Definitions.hpp"

void __stdcall MyD3D11PresentExtension(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags) {
      // code...
}

void __stdcall MyD3D9EndSceneExtension(LPDIRECT3DDEVICE9 pDevice) {
      // code...
}

void MyInitFunction() {
      // Make sure MirrorHook was loaded into the memory.
      HMODULE hMirrorHook = nullptr;
      while (!hMirrorHook) {
         hMirrorHook = GetModuleHandle("MirrorHook.dll");
         Sleep(100);
      }
      
      // Wait while MirrorHook does its thing
      if (MirrorHook::WasAutoInitSuccessful()) {
         switch (MirrorHook::GetInstalledFramework()) {
            case MirrorHook::Framework::D3D9:
               MirrorHook::D3D9::AddExtension(MirrorHook::D3D9::D3D9Extension::EndScene, &MyD3D9EndSceneExtension);
               break;
            case MirrorHook::Framework::D3D11:
               MirrorHook::D3D11::AddExtension(MirrorHook::D3D11::D3D11Extension::Present, &MyD3D11PresentExtension);
               break;
         }
      } else {
         // Initiate MirrorHook by window title
         MirrorHook::PrepareFor(MirrorHook::Framework::*, "window title");
         // or by window handle
         MirrorHook::PrepareFor(MirrorHook::Framework::*, hWnd);
      }
}
```
The actual DirectX libraries are **not needed** to write extensions, `void*` can be used for the parameters instead.
```
// Example:
// Instead of ->
void WINAPI MyD3D9EndSceneExtension(LPDIRECT3DDEVICE9 pDevice);
// You can type ->
void WINAPI MyD3D9EndSceneExtension(void *pDevice);
```

## Development dependencies
- [June 2010 DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- C++17 compliant compiler [e.g., VS2017 and above](https://www.visualstudio.com)
