/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once
#include "emu_typedefs.h"

namespace gb
{
    class GBConsole;

    class Timer
    {
    public:
        Timer(GBConsole* device);
        ~Timer() = default;

        auto read(u16 address) -> u8;
        auto write(u16 address, u8 data) -> void;

        auto clock() -> void;

        inline auto setDIVtoSkippedBootromValue() -> void { internalRegisterDIV = 0xABCC; }

    private:
        GBConsole* system;

        u16 internalRegisterDIV = 0x0000;

        u8 timerCounter = 0x00; // TIMA
        u8 timerModulo = 0x00; // TMA

        union TAC
        {
            struct
            {
                u8 inputClockSelect : 2;
                u8 timerEnable : 1;
                u8 unused : 5;
            };

            u8 reg;
        }timerControl = { };

        //u8 timerControl = 0x00; // TAC

        u8 cyclesElapsed = 0x00;
        u8 lastANDresult = 0x00;
    };
}
