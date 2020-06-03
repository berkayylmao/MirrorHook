# MirrorHook
A wrapper for D3D9, D3D11 and WndProc to let you execute your code before the original implementation.

## How to use
1. Build the project yourself or get the appropriate release build [here](https://github.com/berkayylmao/MirrorHook/releases/latest).
2. Load it into the target process.
3. Include `include/MirrorHook.hpp` in your code.
4. Go to town via `MirrorHook::`.

The release is inject-and-forget. You will practically never need to init MirrorHook yourself. However, in case it does fail, 3 manual methods of initialization exists:
1. `MirrorHook::Init(with window handle)`
2. `MirrorHook::Init(with window title)`
3. `MirrorHook::Init(with directx device pointer)`


## Example implementation:
```cpp
// Example:
#include "path/to/MirrorHook/include/MirrorHook.hpp"
using namespace MirrorHook;

void __stdcall present_D3D11(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags) {
   // code...
   // no need to return any values
}

void __stdcall endscene_D3D9(void *pDevice) {
   // code...
   // no need to return any values
}

void init() {
   D3D9::AddExtension(D3D9::Type::EndScene, &endscene_D3D9);
   D3D11::AddExtension(D3D11::Type::Present, &present_D3D11);
}
```