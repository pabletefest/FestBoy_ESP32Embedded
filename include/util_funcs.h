/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once
#include "emu_typedefs.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#define CONVERT_KB_TO_BYTES(KB) (KB * 1024)

#ifdef _WIN32
#define INLINE __forceinline
#else
#define INLINE inline
#endif

constexpr INLINE u32 convertKBToBytes(u32 KB) { return static_cast<u32>(KB) * 1024; }

template<typename T>
constexpr INLINE T convertKBToBytes(u32 KB) { return static_cast<T>(KB) * 1024; }

constexpr inline unsigned long long operator"" _KB(unsigned long long number) { return 1024ULL * number; }
constexpr inline unsigned long long operator"" _MB(unsigned long long number) { return 1024_KB * number; }
constexpr inline unsigned long long operator"" _GB(unsigned long long number) { return 1024_MB * number; }

template <typename T>
std::vector<T> loadBinary(const std::filesystem::path& path) // const char* path
{
    std::basic_ifstream<T> file{ path, std::ios::binary };
    return { std::istreambuf_iterator<T>{ file }, {} };
}