#pragma once
#include "emu_typedefs.h"

namespace gb
{
    class Mapper
    {
    public:
        Mapper(u8 numROMBanks);
        virtual ~Mapper() = default;

        virtual auto mapRead(u16 addr, u16& mapped_addr) -> bool = 0;
        virtual auto mapWrite(u16 addr, u16& mapped_addr, u8 data) -> bool = 0; // data is provided for mappers that need registers
    
    protected:
        u8 nROMBanks = 0;
    };
}