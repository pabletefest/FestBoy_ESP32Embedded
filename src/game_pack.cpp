/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "game_pack.h"
#include "no_mbc.h"
#include "util_funcs.h"

#include <fstream>
#include <cstring>

gb::GamePak::GamePak(const std::string& filename)
{
    std::memset(&header, 0x00, sizeof(CartridgeHeader));

    std::ifstream ifs;

    ifs.open("/spiffs/" + filename, std::ifstream::binary);

    if (ifs.is_open())
    {
        printf("\nROM file '%s' opened\n", filename.c_str());

        // TEMP: skipping bootrom
        ifs.seekg(256, std::ios_base::cur);

        // Read file header
        ifs.read((char*)&header, sizeof(CartridgeHeader));

        gameName = std::string((const char*)header.title);

        u32 romSize = romSizesTable[header.romSize];
        nROMBanks = static_cast<u8>(romSize / convertKBToBytes(16));

        ifs.seekg(0);
        vROMMemory.resize(nROMBanks * convertKBToBytes(16)); // We could use romSize
        ifs.read((char*)vROMMemory.data(), romSize);

        switch (header.cartridgeType)
        {
        case 0x00:
            mapper = std::make_unique<NoMBCMapper>(nROMBanks);
            break;
        case 0x01:
            //mapper = std::make_unique<NoMBCMapper>(nROMBanks);
            break;
        default:
            break;
        }
    }
    else
    {
        printf("\nROM file '%s' could not be opened\n", filename.c_str());
    }

    printf("ROM buffer size is %d bytes\n", getRomBufferSize());
}

auto gb::GamePak::read(u16 addr, u8& data) -> bool
{
    u16 mappedAddress = 0x0000;

    if (mapper->mapRead(addr, mappedAddress))
    {
        data = vROMMemory[mappedAddress];
        return true;
    }

    return false;
}

auto gb::GamePak::write(u16 addr, u8 data) -> bool
{
    u16 mappedAddress = 0x0000;

    if (mapper->mapWrite(addr, mappedAddress, data))
    {
        // No need to modify ROM memory
        vROMMemory[mappedAddress] = data;
        return true;
    }

    return false;
}

auto gb::GamePak::getHeaderInfo() const -> const CartridgeHeader&
{
    return header;
}

auto gb::GamePak::getROMBuffer() const -> const u8*
{
    return vROMMemory.data();
}

auto gb::GamePak::getRomBufferSize() const -> const u32
{
    return vROMMemory.size() * sizeof(decltype(vROMMemory)::value_type);
}
