/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once
#include "emu_typedefs.h"
#include "mapper.h"

#include <string>
#include <memory>
#include <vector>
#include <array>

namespace gb
{
    // Represents an abstraction of a GB cartridge 
    class GamePak : public std::enable_shared_from_this<GamePak>
    {
    public:
        GamePak(const std::string& filename);
        ~GamePak() = default;

        auto read(u16 addr, u8& data) -> bool;
        auto write(u16 addr, u8 data) -> bool;

        struct CartridgeHeader
        {
            u8 entryPoint[4];
            u8 nintendoLogo[48];
            u8 title[16]; // Special case
            u8 newLicenseCode[2];
            u8 sgbFlag;
            u8 cartridgeType;
            u8 romSize;
            u8 ramSize;
            u8 destinationCode;
            u8 oldLicenseCode;
            u8 maskROMVersionNumber;
            u8 checksum;
            u8 globalChecksum[2];
        };

        auto getHeaderInfo() const -> const CartridgeHeader&;

        auto getROMBuffer() const -> const u8*;
        auto getRomBufferSize() const -> const u32;

    private:
        CartridgeHeader header;

        const std::array<u32, 12> romSizesTable = {
            32 * 1024,
            64 * 1024,
            128 * 1024,
            256 * 1024,
            512 * 1024,
            1 * 1024 * 1024,
            2 * 1024 * 0124,
            4 * 1024 * 1024,
            8 * 1024 * 1024,
            u32(1.1 * 1024 * 1024),
            u32(1.2 * 1024 * 1024),
            u32(1.5 * 1024 * 1024)
        };

        const std::array<u32, 6> ramSizesTable = {
            0,
            0, // Unused
            8 * 1024,
            32 * 1024,
            128 * 1024,
            64 * 1024
        };

        std::string gameName;

        //u8 nMapperID = 0;
        u8 nROMBanks = 0;

        std::vector<u8> vROMMemory;

        Scope<Mapper> mapper;
    };
}
