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
  class VTableHook {
    DWORD_PTR*                          _vt;
    std::unordered_map<UINT, DWORD_PTR> _hookMap;
    std::mutex                          _mutex;

   public:
    template <class Type>
    Type Hook(UINT index, Type fnNew) {
      std::scoped_lock _lock(_mutex);

      DWORD_PTR fnOrig = _vt[index];
      if (!_hookMap.count(index)) {
        DWORD_PTR targetAddr = (DWORD_PTR)&_vt[index];
        Memory::openMemoryAccess(targetAddr, sizeof(DWORD_PTR));
        _vt[index] = (DWORD_PTR)fnNew;
        Memory::restoreMemoryAccess(targetAddr);

        _hookMap.insert(std::make_pair(index, fnOrig));
      }
      return (Type)fnOrig;
    }

    void Unhook(UINT index) {
      std::scoped_lock _lock(_mutex);

      if (_hookMap.count(index)) {
        DWORD_PTR targetAddr = (DWORD_PTR)&_vt[index];
        Memory::openMemoryAccess(targetAddr, sizeof(DWORD_PTR));
        _vt[index] = _hookMap.at(index);
        Memory::restoreMemoryAccess(targetAddr);

        _hookMap.erase(index);
      }
    }
    void UnhookAll() {
      std::scoped_lock _lock(_mutex);
      for (auto const& hook : _hookMap) {
        DWORD_PTR targetAddr = (DWORD_PTR)&_vt[hook.first];
        Memory::openMemoryAccess(targetAddr, sizeof(DWORD_PTR));
        _vt[hook.first] = hook.second;
        Memory::restoreMemoryAccess(targetAddr);
      }

      _hookMap.clear();
    }

    VTableHook(const VTableHook&) = delete;
    VTableHook(PDWORD_PTR* ppClass) { _vt = *ppClass; }
    ~VTableHook() { UnhookAll(); }
  };
}  // namespace Helpers