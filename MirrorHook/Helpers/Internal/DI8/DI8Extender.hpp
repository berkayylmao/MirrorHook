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
#include <memory> // std::unique_ptr, std::make_unique
#include "Helpers/Internal/Memory/Memory.hpp"
#include "Helpers/Internal/Memory/VTableHook.hpp"
#include "Helpers/Internal/DI8/DI8Types.h"
#include "Helpers/Internal/imgui/imgui.h"
#include "Helpers/Internal/imgui/dx9/imgui_impl_dx9.h"
#include "Helpers/Internal/imgui/win32/imgui_impl_win32.h"

#include "inc/Definitions.hpp"
using MirrorHook::DI8::DI8Device;
using MirrorHook::DI8::DI8Extension;

namespace MirrorHookInternals {
   namespace DI8Extender {
      LPDIRECTINPUT8A          di8Instance                       = nullptr;
      LPDIRECTINPUTDEVICE8A    device_Keyboard                   = nullptr;
      LPDIRECTINPUTDEVICE8A    device_Mouse                      = nullptr;

      std::map<DI8Device, std::vector<GetDeviceState_t>> mGetDeviceStateExtensions;

      bool                     isExtenderReady                   = false;

   #pragma region function hooks
      GetDeviceState_t         origGetDeviceState_Keyboard       = nullptr;
      GetDeviceState_t         origGetDeviceState_Mouse          = nullptr;

      HRESULT __stdcall hkGetDeviceState_Keyboard(HINSTANCE hInstance, DWORD cbData, LPVOID lpvData) {
         HRESULT retOrigGetDeviceState = origGetDeviceState_Keyboard(hInstance, cbData, lpvData);

         if (!mGetDeviceStateExtensions[DI8Device::Keyboard].empty()) {
            for (GetDeviceState_t keyboardExtension : mGetDeviceStateExtensions[DI8Device::Keyboard]) {
               if (keyboardExtension)
                  keyboardExtension(hInstance, cbData, lpvData);
            }
         }
         return retOrigGetDeviceState;
      }
      HRESULT __stdcall hkGetDeviceState_Mouse(HINSTANCE hInstance, DWORD cbData, LPVOID lpvData) {
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
      LPDIRECTINPUT8A __stdcall GetDirectInput8A() {
      #pragma ExportedFunction
         if (!isExtenderReady)
            return nullptr;

         return di8Instance;
      }
      LPDIRECTINPUTDEVICE8A __stdcall GetDirectInputDevice8A(DI8Device deviceType) {
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
      bool __stdcall AddExtension(DI8Device deviceType, DI8Extension extensionType, LPVOID extensionAddress) {
      #pragma ExportedFunction
         switch (extensionType) {
            case DI8Extension::GetDeviceState:
            {
               switch (deviceType) {
                  case DI8Device::Keyboard:
                  case DI8Device::Mouse:
                     mGetDeviceStateExtensions[deviceType].push_back(reinterpret_cast<GetDeviceState_t>(extensionAddress));
                     break;
                  default:
                     return false;
               }
               break;
            }
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

      BOOL CALLBACK enumCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID) {
         BYTE deviceType = LOBYTE(lpddi->dwDevType);
         if (deviceType == DI8DEVTYPE_KEYBOARD || deviceType == DI8DEVTYPE_MOUSE) {
            DWORD_PTR* inputTable;

            if (deviceType == DI8DEVTYPE_KEYBOARD) {
               di8Instance->CreateDevice(lpddi->guidInstance, &device_Keyboard, NULL);
               inputTable = *(PDWORD_PTR*)device_Keyboard;
            } else {
               di8Instance->CreateDevice(lpddi->guidInstance, &device_Mouse, NULL);
               inputTable = *(PDWORD_PTR*)device_Mouse;
            }

            Memory::openMemoryAccess(inputTable[9], true, 4);

            if (deviceType == DI8DEVTYPE_KEYBOARD) {
               origGetDeviceState_Keyboard = (GetDeviceState_t)(DWORD_PTR)inputTable[9];
               inputTable[9]               = (DWORD_PTR)hkGetDeviceState_Keyboard;
            } else {
               origGetDeviceState_Mouse    = (GetDeviceState_t)(DWORD_PTR)inputTable[9];
               inputTable[9]               = (DWORD_PTR)hkGetDeviceState_Mouse;
            }

            Memory::restoreMemoryAccess();
         }

         if (device_Keyboard && device_Mouse) {
            isExtenderReady = true;
            return DIENUM_STOP;
         }
         return DIENUM_CONTINUE;
      }
      bool Init(DWORD_PTR dinput8Address) {
         mGetDeviceStateExtensions[DI8Device::Keyboard] = std::vector<GetDeviceState_t>();
         mGetDeviceStateExtensions[DI8Device::Mouse]    = std::vector<GetDeviceState_t>();

         while (!di8Instance) {
            di8Instance = *(LPDIRECTINPUT8A*)(Memory::baseAddress + dinput8Address);
            Sleep(100);
         }
         di8Instance->EnumDevices(DI8DEVCLASS_ALL, &enumCallback, NULL, DIEDFL_ATTACHEDONLY);
         return true;
      }
   }
}