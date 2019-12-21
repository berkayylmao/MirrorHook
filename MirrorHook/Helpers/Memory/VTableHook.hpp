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
#include "Memory.h"
#include <map>

class VTableHook {
   VTableHook(const VTableHook&) = delete;

private:
   DWORD* pOrigVTable;
   std::map<UINT, DWORD> hookedIndexes;

public:
   VTableHook(PDWORD* ppClass) {
      pOrigVTable = *ppClass;
   }

   template<class Type>
   Type Hook(UINT index, Type fnNew) {
      DWORD fnOrig = pOrigVTable[index];
      if (!hookedIndexes.count(index)) {
         Memory::openMemoryAccess(fnOrig, 4);
         pOrigVTable[index] = (DWORD)fnNew;
         Memory::restoreMemoryAccess();

         hookedIndexes.insert(std::make_pair(index, fnOrig));
      }
      return (Type)fnOrig;
   }

   void Unhook(UINT index) {
      if (hookedIndexes.count(index)) {
         Memory::openMemoryAccess(pOrigVTable[index], 4);
         DWORD fnOrig = hookedIndexes.at(index);
         pOrigVTable[index] = fnOrig;
         Memory::restoreMemoryAccess();
         hookedIndexes.erase(index);
      }
   }
   void UnhookAll() {
      for (auto const& hook : hookedIndexes) {
         UINT index = hook.first;
         Memory::openMemoryAccess(pOrigVTable[index], 4);
         pOrigVTable[index] = hook.second;
         Memory::restoreMemoryAccess();
      }
      hookedIndexes.clear();
   }
};