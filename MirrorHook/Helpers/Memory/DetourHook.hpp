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

namespace Helpers {
  class DetourInfo {
    DWORD_PTR address;
    BYTE      originalBytes[5];
    BYTE      hookBytes[5];

   public:
    void Unhook() {
      Memory::openMemoryAccess(address, 5);
      memcpy_s((LPVOID)address, 5, originalBytes, 5);
      Memory::restoreMemoryAccess(address);
    }
    void Hook() {
      Memory::openMemoryAccess(address, 5);
      memcpy_s((LPVOID)address, 5, hookBytes, 5);
      Memory::restoreMemoryAccess(address);
    }

    BYTE* _getOriginalBytes() { return originalBytes; }
    BYTE* _getHookBytes() { return hookBytes; }

    DetourInfo() = default;
    DetourInfo(DWORD_PTR _address) : originalBytes{0x00}, hookBytes{0x00} { address = _address; }
  };

  class DetourHook {
    std::unordered_map<DWORD_PTR, DetourInfo> _detourMap;
    std::mutex                                _mutex;

   public:
    DetourInfo* GetInfoOf(const DWORD_PTR address) {
      std::scoped_lock _lock(_mutex);
      if (_detourMap.count(address)) return &_detourMap[address];

      return nullptr;
    }
    template <class Type>
    DetourInfo* GetInfoOf(const Type fn) {
      return GetInfoOf((DWORD_PTR)fn);
    }

    template <class Type>
    Type Hook(const DWORD_PTR targetAddress, const Type fnNew) {
      std::scoped_lock _lock(_mutex);

      DetourInfo dI(targetAddress);
      Memory::openMemoryAccess(targetAddress, 5);
      memcpy_s(dI._getOriginalBytes(), 5, (LPVOID)targetAddress, 5);
      Memory::writeJMP(targetAddress, (DWORD_PTR)fnNew);
      memcpy_s(dI._getHookBytes(), 5, (LPVOID)targetAddress, 5);
      Memory::restoreMemoryAccess(targetAddress);

      _detourMap.insert(std::make_pair(targetAddress, dI));

      return (Type)targetAddress;
    }
    void Unhook(const DWORD_PTR address) {
      std::scoped_lock _lock(_mutex);
      if (_detourMap.count(address)) {
        _detourMap[address].Unhook();
        _detourMap.erase(address);
      }
    }
    void UnhookAll() {
      std::scoped_lock _lock(_mutex);
      for (auto& dI : _detourMap) dI.second.Unhook();

      _detourMap.clear();
    }

    DetourHook(const DetourHook&) = delete;
    DetourHook()                  = default;
    ~DetourHook() { UnhookAll(); }
  };
}  // namespace Helpers