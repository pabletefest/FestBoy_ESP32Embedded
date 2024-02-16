/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once
#include "mapper.h"

namespace gb
{
    class NoMBCMapper : public Mapper
    {
    public:
        NoMBCMapper(u8 numROMBanks);
        ~NoMBCMapper() override = default;

        // Inherited via Mapper
        virtual auto mapRead(u16 addr, u16& mapped_addr) -> bool override;
        virtual auto mapWrite(u16 addr, u16& mapped_addr, u8 data) -> bool override;
    };
}
