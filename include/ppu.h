/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once
#include "emu_typedefs.h"
#include "util_funcs.h"
#include <array>

#ifdef ESP32
    #include <TFT_eSPI.h>
#endif

namespace gb
{
    class GBConsole;

    class PPU
    {
    public:
        friend class GBConsole;

        // struct Pixel
        // {
        //     // We use this order for convenience
        //     u8 R;
        //     u8 G;
        //     u8 B;

        //     auto operator==(const Pixel& other) -> bool
        //     {
        //         return this->R == other.R && this->G == other.G && this->B == other.B;
        //     }
        // };

    public:
        PPU(GBConsole* device);
        ~PPU() = default;

        auto read(u16 address) -> u8;
        auto write(u16 address, u8 data) -> void;

        auto reset() -> void;
        auto clock() -> void;

        // inline auto getPixelsBufferData() const -> const PPU::Pixel* { return pixelsBuffer.data(); }
        // inline auto getPixelsBuffer() -> std::array<Pixel, 160 * 144>& { return pixelsBuffer; }
        auto getPixelsBufferData() -> u8*;
        auto getSpriteBuffer() -> TFT_eSprite& { return screenSprite; }

    private:
        auto checkAndRaiseStatInterrupts() -> void;
        auto renderBackground() -> void;
        auto renderWindow() -> void;
        auto renderSprites() -> void;
        auto scanlineOAMScanSearchRoutine() -> void;

    public:
        GBConsole* system = nullptr;
        bool frameCompleted = false;

    private:
        TFT_eSPI display;
        TFT_eSprite screenSprite = TFT_eSprite(&display);
        // std::array<Pixel, 160 * 144> pixelsBuffer = {};
        std::array<u8, 8_KB> VRAM = {};
        //std::array<u8, 160> OAM = {};

        struct SpriteInfoOAM
        {
            u8 Yposition;
            u8 Xposition;
            u8 tileIndex;
            u8 attributesFlags;
        };

        std::array<SpriteInfoOAM, 40> OAM = {};
        std::array<SpriteInfoOAM, 10> scanlineValidSprites = {};
        u8 spritesFound = 0;

        u8 LY = 0x00;
        u8 LYC = 0x00;
        u16 currentDot = 0x00;
        u16 remainingDots = 456; // Max dots per scanline
        const u16 totalDotsPerScanline = 456;
        u16 lastMode3Dot = 0;
        u8 SCX = 0x00;
        u8 SCY = 0x00;

        union LCDC
        {
            struct
            {
                u8 BGWindEnablePriority : 1;
                u8 OBJenable : 1;
                u8 OBJsize : 1;
                u8 BGtileMapArea : 1;
                u8 BGWindTileDataArea : 1;
                u8 WindowEnable : 1;
                u8 WindowTileMapArea : 1;
                u8 LCDenable : 1;
            };

            u8 reg;
        }LCDControl = {};

        union STAT
        {
            struct
            {
                u8 ModeFlag : 2;
                u8 LYCLY_Flag : 1;
                u8 Mode0STATIntrSrc : 1;
                u8 Mode1STATIntrSrc : 1;
                u8 Mode2STATIntrSrc : 1;
                u8 LYCLYSTATIntrSrc : 1;
                u8 unused : 1;
            };

            u8 reg;
        }LCDStatus = {};

        u8 bgPaletteData = 0x00;
        u8 obj0PaletteData = 0x00;
        u8 obj1PaletteData = 0x00;
    };
}
