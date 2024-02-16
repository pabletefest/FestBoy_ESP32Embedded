/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "timer.h"
#include "gb.h"

static constexpr u32 CPU_CLOCK_SPEED = 4194304u;
static constexpr u32 timaClockSpeeds[4] = { 4096u, 262144u, 65536u, 16384u };
static constexpr u8 watchableInternalDIVbits[4] = { 9, 3, 5, 7 };

gb::Timer::Timer(GBConsole* device)
    : system(device), timerControl({})
{
}

auto gb::Timer::read(u16 address) -> u8
{
    u8 dataRead = 0x00;

	switch (address)
	{
    case 0xFF04:
        dataRead = (internalRegisterDIV >> 8) & 0x00FF;
        break;
    case 0xFF05:
        dataRead = timerCounter;
        break;
    case 0xFF06:
        dataRead = timerModulo;
        break;
    case 0xFF07:
        dataRead = timerControl.reg;
        break;
	}

    return dataRead;
}

auto gb::Timer::write(u16 address, u8 data) -> void
{
    switch (address)
    {
    case 0xFF04:
        internalRegisterDIV = 0x0000;
        break;
    case 0xFF05:
        timerCounter = data;
        break;
    case 0xFF06:
        timerModulo = data;
        break;
    case 0xFF07:
        timerControl.reg = data;
        break;
    }
}

auto gb::Timer::clock() -> void
{
    internalRegisterDIV++;

    u8 watchedBit = (internalRegisterDIV >> watchableInternalDIVbits[timerControl.inputClockSelect]) & 1u;
    u8 timerEnable = timerControl.timerEnable & 1u;
    u8 AND_Result = watchedBit & timerEnable;

    if (lastANDresult == 1 && AND_Result == 0) // Falling edge occured
    {
        timerCounter++;

        if (timerCounter == 0x00) // TIMA overflowed
        {
            timerCounter = timerModulo;
            system->requestInterrupt(gb::GBConsole::InterruptType::Timer);
        }
    }

    lastANDresult = AND_Result;

    //if (timerControl.timerEnable)
    //{
    //    if (cyclesElapsed == (CPU_CLOCK_SPEED / timaClockSpeeds[timerControl.inputClockSelect]))
    //    {
    //        if (timerCounter == 0xFF)
    //        {
    //            timerCounter = timerModulo;
    //            system->requestInterrupt(gb::GBConsole::InterruptType::Timer);
    //        }
    //        else
    //        {
    //            timerCounter++;
    //        }
    //    }
    //    else
    //    {
    //        cyclesElapsed++;
    //    }
    //}
}
