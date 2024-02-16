/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <memory>
#include <cstdint>

using u8 = std::uint8_t;
using s8 = std::int8_t;
using i8 = s8;
using u16 = std::uint16_t;
using s16 = std::int16_t;
using i16 = s16;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using i32 = s32;

using vu8 = volatile std::uint8_t;
using vs8 = volatile std::int8_t;
using vi8 = vs8;
using vu16 = volatile std::uint16_t;
using vs16 = volatile std::int16_t;
using vi16 = vs16;
using vu32 = volatile std::uint32_t;
using vs32 = volatile std::int32_t;
using vi32 = vs32;

using f32 = float;

template<typename T> using Scope = std::unique_ptr<T>;
template<typename T> using Unique = std::unique_ptr<T>;
template<typename T> using Ref = std::shared_ptr<T>;
template<typename T> using Shared = std::shared_ptr<T>;
