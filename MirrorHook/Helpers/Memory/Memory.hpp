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

namespace Helpers::Memory {
  struct MemoryAccessInfo {
    DWORD   oldMemoryAccess;
    int32_t size;

    MemoryAccessInfo() = default;
    MemoryAccessInfo(int32_t _size) : oldMemoryAccess(NULL) { size = _size; }
  };
  static inline std::map<DWORD_PTR, MemoryAccessInfo> _accessMap;
  static inline std::mutex                            _memoryMutex;

  static void openMemoryAccess(const DWORD_PTR address, const int size) {
    MemoryAccessInfo mA(size);
    VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &mA.oldMemoryAccess);

    _memoryMutex.lock();
    _accessMap.insert(std::make_pair(address, mA));
    _memoryMutex.unlock();
  }
  static void restoreMemoryAccess(const DWORD_PTR address) {
    MemoryAccessInfo* pMA = &_accessMap[address];
    VirtualProtect((LPVOID)address, pMA->size, pMA->oldMemoryAccess, &pMA->oldMemoryAccess);
    pMA = nullptr;

    _memoryMutex.lock();
    _accessMap.erase(address);
    _memoryMutex.unlock();
  }

  static void writeJMP(const DWORD_PTR from, const DWORD_PTR to) {
    DWORD32 relativeAddress = to - from - 0x5;
    openMemoryAccess(from, 5);
    *(BYTE*)(from)          = 0xE9;
    *(DWORD32*)(from + 0x1) = relativeAddress;
    restoreMemoryAccess(from);
  }
}  // namespace Helpers::Memory