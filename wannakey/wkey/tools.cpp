// Copyright 2017 Adrien Guinet <adrien@guinet.me>
// This file is part of wannakey.
// 
// wannakey is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// wannakey is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with wannakey.  If not, see <http://www.gnu.org/licenses/>.

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <Windows.h>
#endif

#include <array>
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <string>
#include <cmath>
#include <algorithm>
#include <memory.h>

#include <wkey/tools.h>

namespace wkey {

void dumpHex(const char* Name, uint8_t const* Data, size_t const Len)
{
  printf("%s:", Name);
  for (size_t i = 0; i < Len; ++i) {
    if ((i % 16 == 0)) {
      printf("\n");
    }
    printf("%02X ", Data[i]);

  }
  printf("\n====\n");
}

double normalizedEntropy(uint8_t const* Data, const size_t Len)
{
  // Initialized at 0 thanks to the uint32_t constructor.
  std::array<uint32_t, 256> Hist;

  std::fill(Hist.begin(), Hist.end(), 0);

  for (size_t i = 0; i < Len; ++i) {
    ++Hist[Data[i]];
  }

  double Ret = 0.0;
  for (uint32_t Count : Hist) {
    if (Count) {
      double const P = (double)Count / (double)Len;
      Ret += P * std::log(P);
    }
  }
  if (Ret == 0.0) {
    // Or we would have -0.0 with the line below!
    return 0.0;
  }

  return -Ret / std::log(256.);
}

std::error_code getLastErrno()
{
  return std::error_code{ errno, std::system_category() };
}

#ifdef _WIN32
std::string getLastErrorMsg()
{
  return getLastEC().message();
}

std::error_code getLastEC()
{
  return std::error_code{ (int)GetLastError(), std::system_category() };
}
#endif

uint8_t const* memmem(const uint8_t *haystack, size_t hlen, const uint8_t *needle, size_t nlen)
{
	if (nlen == 0) {
		return NULL;
	}
	uint8_t const* curPtr = haystack;
	uint8_t const* const endPtr = haystack + hlen;
	uint8_t const firstByte = *needle;
	do {
		curPtr = std::find(curPtr, endPtr, firstByte);
		if (curPtr < endPtr) {
			if (((uintptr_t)endPtr-(uintptr_t)curPtr) < nlen) {
				return nullptr;
			}
			if (memcmp(curPtr, needle, nlen) == 0) {
				return curPtr;
			}
		}
		++curPtr;
	} while (curPtr < endPtr);

	return nullptr;
}


std::vector<uint8_t> readFile(const char* path, std::error_code& EC)
{
  std::vector<uint8_t> Ret;
  FILE* f = fopen(path, "rb");
  if (!f) {
    EC = getLastErrno();
    return Ret;
  }
  fseek(f, 0, SEEK_END);
  const long Size = ftell(f);
  fseek(f, 0, SEEK_SET);
  Ret.resize(Size);
  if (fread(&Ret[0], 1, Size, f) != Size) {
    EC = getLastErrno();
    return Ret;
  }
  EC = std::error_code{};
  return Ret;
}

bool fileHasString(const char* path, const char* str)
{
  std::error_code EC;
  auto Data = readFile(path, EC);
  if (EC) {
    std::cerr << "error reading '" << path << "': " << EC.message() << std::endl;
    return false;
  }
  return memmem(&Data[0], Data.size(), (const uint8_t*)str, strlen(str)) != NULL;
}

} // wkey
