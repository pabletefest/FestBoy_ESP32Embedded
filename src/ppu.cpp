/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.h"
#include "gb.h"

#include <cstring>

#define GB_PIXELS_WIDTH 160
#define GB_PIXELS_HEIGHT 144

namespace gb
{
    // "$8800 (LCD Control bit 4 is 0) and $8000 (LCD Control bit 4 is 1) addressing modes to access BG and Window Tile Data"
    static constexpr u16 vramAddressingMode[2][2] = { { 0x9000, 0x8800 }, { 0x8000, 0x8800 } };
    
    // When LCD Control bit 6 and/or 3 are set, tilemap base address is 0x9C00, 0x9800 otherwise.
    static constexpr u16 tileMapAddress[2] = { 0x9800, 0x9C00 };

    enum PixelBBP
    {
        INVALID_BPP = 0, BBP1 = 1, BBP4 = 4, BBP8 = 8, BBP16 = 16
    };

    #define RGB888_TO_RGB332(R,G,B) (((R >> 5) << 5) | ((G >> 5) << 2) | (B >> 6))
    #define RGB888_TO_RGB565(R,G,B) (((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3))

    static constexpr u8 greenShadesRGB332Palette[4] = { RGB888_TO_RGB332(155, 188, 15), RGB888_TO_RGB332(139, 172, 15), RGB888_TO_RGB332(48, 98, 48), RGB888_TO_RGB332(15, 56, 15) };
    static constexpr u8 greyShadesRGB332Palette[4] = { RGB888_TO_RGB332(255, 255, 255), RGB888_TO_RGB332(169, 169, 169), RGB888_TO_RGB332(84, 84, 84), RGB888_TO_RGB332(0, 0, 0) };

    static constexpr u16 greenShadesRGB565Palette[4] = { RGB888_TO_RGB565(155, 188, 15), RGB888_TO_RGB565(139, 172, 15), RGB888_TO_RGB565(48, 98, 48), RGB888_TO_RGB565(15, 56, 15) };
    static constexpr u16 greyShadesRGB565Palette[4] = { RGB888_TO_RGB565(255, 255, 255), RGB888_TO_RGB565(169, 169, 169), RGB888_TO_RGB565(84, 84, 84), RGB888_TO_RGB565(0, 0, 0) };

    static constexpr u8 TILES_PER_LINE = 20;
    static constexpr u8 NUMBER_OF_TILE_LINES = 18;
    static constexpr u8 PIXELS_PER_LINE = 160;
    static constexpr u8 NUMBER_OF_LINES = 144;
}

gb::PPU::PPU(GBConsole* device)
    : system(device), LCDControl({}), LCDStatus({})
{
    display.init();
    display.setRotation(1);
    display.resetViewport();
    display.fillScreen(TFT_BLACK);
    screenSprite.setRotation(1);
    screenSprite.resetViewport();
    screenSprite.setColorDepth(4);
    screenSprite.createSprite(GB_PIXELS_WIDTH * 2, GB_PIXELS_HEIGHT * 2);
    screenSprite.fillScreen(TFT_BLACK);
    screenSprite.createPalette(greenShadesRGB565Palette, 4);
    // std::memset(VRAM.data(), 0x00, VRAM.size());
    std::memset(OAM.data(), 0x00, OAM.size() * sizeof(SpriteInfoOAM));
    std::memset(scanlineValidSprites.data(), 0x00, scanlineValidSprites.size() * sizeof(SpriteInfoOAM));
}

auto gb::PPU::read(u16 address) -> u8
{
    u8 dataRead = 0x00;

    if (address >= 0x8000 && address <= 0x9FFF)
    {
        /*if (LCDStatus.ModeFlag == 3)
            return 0xFF;*/

        dataRead = VRAM[address & 0x1FFF];
    }
    else if (address >= 0xFE00 && address <= 0xFE9F)
    {
        /*if (LCDStatus.ModeFlag == 3 || LCDStatus.ModeFlag == 2)
            return 0xFF;*/

        dataRead = reinterpret_cast<u8*>(OAM.data())[address & 0x9F];
    }
    else
    {
        switch (address)
        {
        case 0xFF40:
            dataRead = LCDControl.reg;
            break;
        case 0xFF41:
            dataRead = LCDStatus.reg | 0x80; // No bit 7 so always read as a 1
            break;
        case 0xFF42:
            dataRead = SCY;
            break;
        case 0xFF43:
            dataRead = SCX;
            break;
        case 0xFF44:
            if (!LCDControl.LCDenable)
                LY = 0x00;

            dataRead = LY;
            break;
        case 0xFF45:
            dataRead = LYC;
            break;
        case 0xFF47:
            dataRead = bgPaletteData;
            break;
        case 0xFF48:
            dataRead = obj0PaletteData;
            break;
        case 0xFF49:
            dataRead = obj1PaletteData;
            break;
        }
    }

    return dataRead;
}

auto gb::PPU::write(u16 address, u8 data) -> void
{
    if (address >= 0x8000 && address <= 0x9FFF)
    {
        /*if (LCDStatus.ModeFlag == 3)
            return;*/

        VRAM[address & 0x1FFF] = data;
    }
    else if (address >= 0xFE00 && address <= 0xFE9F)
    {
        /*if (LCDStatus.ModeFlag == 3 || LCDStatus.ModeFlag == 2)
            return;*/

        reinterpret_cast<u8*>(OAM.data())[address & 0x9F] = data;
    }
    else
    {
        switch (address)
        {
        case 0xFF40:
            LCDControl.reg = data;
            break;
        case 0xFF41:
            LCDStatus.reg |= (data & 0x78); // Only bits 6, 5, 4, and 3 are writable
            LCDStatus.unused = 1;
            break;
        case 0xFF42:
            SCY = data;
            break;
        case 0xFF43:
            SCX = data;
            break;
        case 0xFF45:
            LYC = data;
            if (LYC == LY)
                LCDStatus.LYCLY_Flag = 1;
            else
                LCDStatus.LYCLY_Flag = 0;
            checkAndRaiseStatInterrupts();
            break;
        case 0xFF47:
            bgPaletteData = data;
            break;
        case 0xFF48:
            obj0PaletteData = data;
            break;
        case 0xFF49:
            obj1PaletteData = data;
            break;
        }
    }
}

auto gb::PPU::reset() -> void
{
}

auto gb::PPU::clock() -> void
{
    if (!LCDControl.LCDenable)
        return;

    //checkAndRaiseStatInterrupts();
   
    if (LY >= 0 && LY <= 143)
    {
        if (currentDot == 0)
        {
            if (LYC == LY)
                LCDStatus.LYCLY_Flag = 1;
            else
                LCDStatus.LYCLY_Flag = 0;

            checkAndRaiseStatInterrupts();
        }

        // Mode 2 (OAM search)
        if (currentDot >= 0 && currentDot <= 79)
        {
            if (currentDot == 0)
            {
                LCDStatus.ModeFlag = 2;
                lastMode3Dot = 172 + 80 - 1; // Min number of dots is 168-174 according to different sources (172 placeholder for now)
                // Serial.println("Mode 2 entered");
            }

            if (currentDot == 79) // Checking for valid objects in the current scanline performed in the last cycle of mode 2
                scanlineOAMScanSearchRoutine();

            //checkAndRaiseStatInterrupts();
        }

        // Mode 3 (Rendering picture)
        if (currentDot >= 80 && currentDot <= lastMode3Dot)
        {
            if (currentDot == 80)
            {
                LCDStatus.ModeFlag = 3;
                //checkAndRaiseStatInterrupts();
                // Serial.println("Mode 3 entered");
            }

            // Render the line in the last dot before HBlank (scanline renderer)
            if (currentDot == lastMode3Dot)
            {
                if (LCDControl.BGWindEnablePriority)
                {
                    renderBackground();
                    renderWindow();
                }

                if (LCDControl.OBJenable)
                    renderSprites();
            }
        }

        // Mode 0 (HBlank period)
        //if ((currentDot >= (lastMode3Dot + 1)) && (currentDot <= (remainingDots - 1)))
        if ((currentDot >= (lastMode3Dot + 1)) && (currentDot <= (totalDotsPerScanline - 1)))
        {
            if (currentDot == (lastMode3Dot + 1))
            {
                LCDStatus.ModeFlag = 0;
                //checkAndRaiseStatInterrupts();
                // Serial.println("Mode 0 entered");
            }
        }
    }
    else
    {
        // Mode 1 (VBlank period)
        if (LY == 144 && currentDot == 0)
        {
            LCDStatus.ModeFlag = 1;
            system->requestInterrupt(gb::GBConsole::InterruptType::VBlank);
            // Serial.println("Mode 0 entered");
        }
    }

    currentDot++;
    remainingDots--;

    if (currentDot == 456)
    {
        currentDot = 0;
        remainingDots = 456;
        LY++;
        // Serial.printf("LY: %d\n", LY);

        if (LY == 154)
        {
            LY = 0;
            frameCompleted = true;
            // static unsigned frameCount = 0;
            // Serial.printf("Frame #%d completed!\n", frameCount++);
        }
    }
}

auto gb::PPU::getPixelsBufferData() -> u8 *
{
    return reinterpret_cast<u8 *>(getSpriteBuffer().getPointer());
}

auto gb::PPU::checkAndRaiseStatInterrupts() -> void
{
    // STAT Interrupt only triggered in rising edge, that is from low 0 to high 1
    if (!system->getInterruptState(gb::GBConsole::InterruptType::STAT))
    {
        if ((LCDStatus.LYCLY_Flag && LCDStatus.LYCLYSTATIntrSrc) ||
            (LCDStatus.ModeFlag == 0 && LCDStatus.Mode0STATIntrSrc) ||
            (LCDStatus.ModeFlag == 2 && LCDStatus.Mode2STATIntrSrc) ||
            (LCDStatus.ModeFlag == 1 && LCDStatus.Mode1STATIntrSrc))
        {
            system->requestInterrupt(gb::GBConsole::InterruptType::STAT);
        }
    }
}

auto gb::PPU::renderBackground() -> void
{
    u16 tileLine = 32 * (((LY + SCY) & 0xFF) / 8);
    u16 tileY = (LY + SCY) % 8;
    u16 tileOffset = (0 + tileLine) & 0x3FF;
    const u16* addressingMode = vramAddressingMode[LCDControl.BGWindTileDataArea];
    u16 bgTileMapAddress = tileMapAddress[LCDControl.BGtileMapArea] + tileOffset;

    for (int tileIndex = 0; tileIndex < TILES_PER_LINE; tileIndex++)
    {
        u8 tileId = read(bgTileMapAddress);
        u16 tileDataAddress = *addressingMode;

        if (LCDControl.BGWindTileDataArea == 0)
            tileDataAddress += static_cast<s8>(tileId) * 16;
        else
            tileDataAddress += tileId * 16;

        tileDataAddress += (tileY * 2);
        u8 lowByteTileData = read(tileDataAddress);
        u8 highByteTileData = read(tileDataAddress + 1);

        for (int pixelIndex = 0; pixelIndex < 8; pixelIndex++)
        {
            u8 lowBit = (lowByteTileData >> (7 - pixelIndex)) & 1;
            u8 highBit = (highByteTileData >> (7 - pixelIndex)) & 1;
            u8 paletteColorIndex = ((highBit << 1) | lowBit) & 0b11;
            u8 colorPixel = (bgPaletteData >> (paletteColorIndex * 2)) & 0b11;

            u8 y = LY;
            u8 x = (tileIndex * 8 + pixelIndex);
            // std::size_t bufferIndex = (LY * PIXELS_PER_LINE) + (tileIndex * 8 + pixelIndex);

            // u8* pixelsBuffer = reinterpret_cast<u8*>(screenSprite.getPointer());

            switch(screenSprite.getColorDepth())
            {
            case BBP1:
                break;
            case BBP4:
            {
                u8 colorIndex = colorPixel & 0b11;
                screenSprite.drawPixel(x * 2, y * 2, colorIndex);
                screenSprite.drawPixel(x * 2 + 1, y * 2, colorIndex);
                screenSprite.drawPixel(x * 2, y * 2 + 1, colorIndex);
                screenSprite.drawPixel(x * 2 + 1, y * 2 +1, colorIndex);
            }
                break;
            case BBP8:
            {
                u8 paletteColor = (LCDControl.BGWindEnablePriority) ? greenShadesRGB332Palette[colorPixel & 0b11] : greenShadesRGB332Palette[0];
                // pixelsBuffer[bufferIndex] = std::move(paletteColor);
                screenSprite.drawPixel(x, y, paletteColor);
            }
                break;
            case BBP16:
            {
                u16 paletteColor = (LCDControl.BGWindEnablePriority) ? greenShadesRGB565Palette[colorPixel & 0b11] : greenShadesRGB565Palette[0];
                screenSprite.drawPixel(x, y, paletteColor);
            }
                break;
            case INVALID_BPP:
            default:
                break;
            }
        }

        bgTileMapAddress++;
    }
}

auto gb::PPU::renderWindow() -> void
{
}

auto gb::PPU::renderSprites() -> void
{
    for (int item = spritesFound - 1; item >= 0; item--)
    {
        const auto& obj = scanlineValidSprites[item];

        u8 tileIndex = LCDControl.OBJsize ? (obj.tileIndex & 0xFE) : obj.tileIndex;
        u8 tileDataYOffset = (LY + 16 - obj.Yposition) * 2;
        u16 tileDataAddress = 0x8000 + (tileIndex * 16) + tileDataYOffset;
        u8 lowByteTileData = read(tileDataAddress);
        u8 highByteTileData = read(tileDataAddress + 1);

        u8 initialPixelIndex = 0;
        u8 finalPixelIndex = 8;

        if ((obj.Xposition - 8) < 0)
            initialPixelIndex = 8 - (obj.Xposition % 8);
        else if (obj.Xposition >= 160)
            finalPixelIndex = 8 - (obj.Xposition % 8);

        for (int pixelIndex = initialPixelIndex; pixelIndex < finalPixelIndex; pixelIndex++)
        {
            u8 lowBit = (lowByteTileData >> (7 - pixelIndex)) & 1;
            u8 highBit = (highByteTileData >> (7 - pixelIndex)) & 1;
            u8 paletteColorIndex = ((highBit << 1) | lowBit) & 0b11;
            u8 colorPixel = (((obj.attributesFlags & 0x10) ? obj1PaletteData : obj0PaletteData) >> (paletteColorIndex * 2)) & 0b11;

            if (colorPixel == 0)
                continue;

            u8 y = LY;
            u8 x = (obj.Xposition - 8 + pixelIndex);
            // std::size_t bufferIndex = (LY * PIXELS_PER_LINE) + (obj.Xposition - 8 + pixelIndex);

            // u8* pixelsBuffer = reinterpret_cast<u8*>(screenSprite.getPointer());

            u8 colorDepth = screenSprite.getColorDepth();
            u16 bgColor = greenShadesRGB565Palette[0];

            if (colorDepth == BBP8)
                bgColor = greenShadesRGB332Palette[0] & 0x00FF;

            if ((obj.attributesFlags & 0x80) && (screenSprite.readPixelValue(x, y) != bgColor))
                continue;

            switch(colorDepth)
            {
            case BBP1:
                break;
            case BBP4:
            {
                u8 colorIndex = colorPixel & 0b11;
                screenSprite.drawPixel(x * 2, y * 2, colorIndex);
                screenSprite.drawPixel(x * 2 + 1, y * 2, colorIndex);
                screenSprite.drawPixel(x * 2, y * 2 + 1, colorIndex);
                screenSprite.drawPixel(x * 2 + 1, y * 2 +1, colorIndex);
            }       
            break;
            case BBP8:
            {
                u8 paletteColor = greenShadesRGB332Palette[colorPixel & 0b11];
                // pixelsBuffer[bufferIndex] = std::move(paletteColor);
                screenSprite.drawPixel(x, y, paletteColor);
            }
                break;
            case BBP16:
            {
                u16 paletteColor = greenShadesRGB565Palette[colorPixel & 0b11];              
                screenSprite.drawPixel(x, y, paletteColor);
            }
                break;
            case INVALID_BPP:
            default:
                break;
            }
        }
    }
}

auto gb::PPU::scanlineOAMScanSearchRoutine() -> void
{
    u8 spriteSize = 8 << LCDControl.OBJsize; // 8x8 (OBJsize is 0) or 8x16 (OBJsize is 1) sprites
    spritesFound = 0;

    for (const auto& objItem : OAM)
    {
        if (spritesFound == 10)
            break;

        if ((objItem.Yposition <= (LY + 16)) && ((LY + 16) < (objItem.Yposition + spriteSize)))
        {
            scanlineValidSprites[spritesFound++] = objItem;
        }
    }
}

auto gb::PPU::drawFrameToDisplay() -> void
{
    // screenSprite.fillSprite(TFT_RED);

    // for (int y = 0; y < 144; y++)
    // {
    //     for (int x = 0; x < 160; x++)
    //     {
    //         screenSprite.drawPixel(x, y, greenShadesRGB565Palette[x%4]);
    //     }
    // }

    // Serial.printf("Width: %d - Height: %d\n", screenSprite.width(), screenSprite.height());

    u16 x = display.width() / 2 - screenSprite.width() / 2;
    u16 y = display.height() / 2 - screenSprite.height() / 2;

    if (screenSprite.getColorDepth() == BBP4)
        y += (x / y) * 2; // Adding some padding between the texts and the sprite

    screenSprite.pushSprite(x, y);
    // screenSprite.pushSprite(display.width() / 2 - GB_PIXELS_WIDTH / 2, display.height() / 2 - GB_PIXELS_HEIGHT / 2);
    // screenSprite.pushSprite(0, 0);
}

auto gb::PPU::printTextToDisplay(const std::string& text, u8 font, u8 datum) -> void
{
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    // display.setTextFont(font);
    display.setTextDatum(datum);
    // display.setCursor(0, 0);
    display.println(text.c_str());
}

auto gb::PPU::printTextToDisplay(const std::string& text, u16 x, u16 y, u8 font, u8 datum) -> void
{
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    // display.setTextFont(font);
    display.setTextDatum(datum);
    // display.setCursor(0, 0);
    display.drawString(text.c_str(), x, y, font);
}