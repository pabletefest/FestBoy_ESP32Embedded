/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "no_mbc.h"

gb::NoMBCMapper::NoMBCMapper(u8 numROMBanks)
    : Mapper(numROMBanks)
{
}

auto gb::NoMBCMapper::mapRead(u16 addr, u16& mapped_addr) -> bool
{
    if (addr >= 0x0000 && addr <= 0x7FFF)
    {
        mapped_addr = addr;
        return true;
    }

    return false;
}

auto gb::NoMBCMapper::mapWrite(u16 addr, u16& mapped_addr, u8 data) -> bool
{
    /*if (addr >= 0x0000 && addr <= 0x7FFF)
    {
        mapped_addr = addr;
        return true;
    }*/

    return false;
}
