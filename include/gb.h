/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once
#include "emu_typedefs.h"
#include "util_funcs.h"
#include "cpu_sm83.h"
#include "game_pack.h"
#include "timer.h"
#include "ppu.h"

#include <array>

namespace gb
{
    class GBConsole
    {
    public:
        enum class InterruptType
        {
            VBlank, STAT, Timer, Serial, Joypad
        };

    public:
        GBConsole();
        ~GBConsole() = default;

        auto insertCartridge(const Ref<GamePak>& cartridge) -> void;

        auto read8(const u16& address) -> u8;
        auto read16(const u16& address) -> u16;

        auto write8(const u16& address, const u8& data) -> void;
        auto write16(const u16& address, const u16& data) -> void;

        auto reset() -> void;
        auto clock() -> void;
        auto step(u32 numberCycles) -> void;

        inline auto getCPU() -> SM83CPU& { return cpu;  }
        inline auto getTimer() -> Timer& { return timer; }
        inline auto getPPU() -> PPU& { return ppu; }

        auto requestInterrupt(InterruptType type) -> void;
        auto getInterruptState(InterruptType type) -> u8;
        inline auto checkPendingInterrupts() -> u8 { return IE.reg & IF.reg & 0x1F; }
        inline auto enterHaltMode() -> void { isHaltMode = true; }

        auto getGameTitleFromHeader() -> std::string;

    private:
        auto skipBootROM() -> void;

    private:
        SM83CPU cpu;
        std::array<u8, convertKBToBytes(8)> wram;
        std::array<u8, 127> hram;

        u32 systemCyclesElapsed = 0;

        Ref<GamePak> gamePak;

        u8 SB_register = 0x00; // Serial transfer data register
        u8 SC_register = 0x00; // Serial transfer control register

        Timer timer;
        bool isHaltMode = false;

        PPU ppu;

        u8 bootROMMappedRegister = 0x00;
        
        u8 dmaSourceAddress = 0x00;

    public:
        bool IME = false;
        bool pendingInterrupt = false;

        union InterruptEnableRegister
        {
            struct
            {
                u8 VBlank : 1;
                u8 LCD_STAT : 1;
                u8 Timer : 1;
                u8 Serial : 1;
                u8 Joypad : 1;
                u8 unused : 3;
            };

            u8 reg;
        }IE;

        union InterruptFlagsRegister
        {
            struct
            {
                u8 VBlank : 1;
                u8 LCD_STAT : 1;
                u8 Timer : 1;
                u8 Serial : 1;
                u8 Joypad : 1;
                u8 unused : 3;
            };

            u8 reg;
        }IF;

        u8 joypadRegister = 0xCF;

        union
        {
            struct
            {
                u8 buttons : 4;
                u8 dpad : 4;
            };
            u8 _ = 0xFF;
        }controllerState;
    };
}