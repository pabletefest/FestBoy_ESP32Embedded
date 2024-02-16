/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "gb.h"
#include "instructions.h"

gb::SM83CPU::SM83CPU(GBConsole* device)
    : system(device), regs({})
{
}

auto gb::SM83CPU::read8(const u16& address) -> u8
{
    return system->read8(address);
}

auto gb::SM83CPU::read16(const u16& address) -> u16
{
    return system->read16(address);
}

auto gb::SM83CPU::write8(const u16& address, const u8& data) -> void
{
    system->write8(address, data);
}

auto gb::SM83CPU::write16(const u16& address, const u16& data) -> void
{
#ifdef TESTING
    //reinterpret_cast<u16*>(debugRAM)[address] = data;
    write8(address, static_cast<u8>(data & 0x00FF));
    write8(address + 1, static_cast<u8>((data >> 8) & 0x00FF));
    return;
#endif 

    system->write16(address, data);
}

auto gb::SM83CPU::reset() -> void
{
    regs.PC = 0x0000;
    // Initial DMG register values after bootrom
    //setRegisterValuesPostBootROM();
}

auto gb::SM83CPU::clock() -> void
{
    if (instructionCycles == 0) // Time to fetch and execute next opcode
    {
        if (system->IME && (system->IF.reg & system->IE.reg & 0x1F))
        {
            instructionCycles = interruptServiceRoutine();
        }
        else
        {
            if (interruptEnablePending)
            {
                interruptEnablePending = false;
                system->IME = true;
            }

            u8 opcode = read8(regs.PC++);
            instructionCycles = instructionsCyclesTable[opcode];
            decodeAndExecuteInstruction(opcode);
        }
    }

    if (instructionCycles > 0) 
        instructionCycles--;

    cpuT_CyclesElapsed++;
    cpuM_CyclesElapsed = cpuT_CyclesElapsed / 4;
}

auto gb::SM83CPU::checkPendingInterrupts() -> bool
{
    return (system->IE.VBlank & system->IF.VBlank)
        || (system->IE.LCD_STAT & system->IF.LCD_STAT)
        || (system->IE.Timer & system->IF.Timer)
        || (system->IE.Serial & system->IF.Serial)
        || (system->IE.Joypad & system->IF.Joypad);
}

auto gb::SM83CPU::interruptServiceRoutine() -> u8
{
    read8(regs.PC++); // Cancelled fetch

    regs.PC--; // Amend the PC increment after cancelled opcode fetch

    write8(--regs.SP, (regs.PC >> 8) & 0x00FF);

    write8(--regs.SP, regs.PC & 0x00FF);

    u8 interrupts = system->IF.reg & system->IE.reg & 0x1F;

    if (interrupts & 0x01) // VBlank
    {
        system->IF.reg &= ~0x01;
        regs.PC = 0x0040;
    }
    else if (interrupts & 0x02) // STAT
    {
        system->IF.reg &= ~0x02;
        regs.PC = 0x0048;
    }
    else if (interrupts & 0x04) // Timer
    {
        system->IF.reg &= ~0x04;
        regs.PC = 0x0050;
    }
    else if (interrupts & 0x08) // Serial
    {
        system->IF.reg &= ~0x08;
        regs.PC = 0x0058;
    }
    else if (interrupts & 0x10) // Joypad
    {
        system->IF.reg &= ~0x10;
        regs.PC = 0x0060;
    }

    system->IME = false;

    return 20; //ISP takes 5 m-cycles (20 t-cycles)
}

auto gb::SM83CPU::decodeAndExecuteInstruction(u8 opcode) -> void
{
    switch (opcode)
    {
    case 0x00:
        NOP();
        break;
    case 0x01:
        LD<REGISTER, IMMEDIATE, u16>(this, regs.BC, read16(regs.PC));
        regs.PC += 2;
        break;
    case 0x02:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.BC, regs.A);
        break;
    case 0x03:
        INC<REGISTER, u16>(this, regs.BC);
        break;
    case 0x04:
        INC<REGISTER, u8>(this, regs.B);
        break;
    case 0x05:
        DEC_<REGISTER, u8>(this, regs.B);
        break;
    case 0x06:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.B, read8(regs.PC++));
        break;
    case 0x07:
        RLCA(this);
        break;
    case 0x08:
        {
            u16 address = read16(regs.PC);
            LD_u16SP(this, address);
            regs.PC += 2;
        }
        break;
    case 0x09:
        ADD_HLrr(this, regs.BC);
        break;
    case 0x0A:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(regs.BC));
        break;
    case 0x0B:
        DEC_<REGISTER, u16>(this, regs.BC);
        break;
    case 0x0C:
        INC<REGISTER, u8>(this, regs.C);
        break;
    case 0x0D:
        DEC_<REGISTER, u8>(this, regs.C);
        break;
    case 0x0E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.C, read8(regs.PC++));
        break;
    case 0x0F:
        RRCA(this);
        break;
    case 0x10:
        // TODO: STOP instruction
        break;
    case 0x11:
        LD<REGISTER, IMMEDIATE, u16>(this, regs.DE, read16(regs.PC));
        regs.PC += 2;
        break;
    case 0x12:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.DE, regs.A);
        break;
    case 0x13:
        INC<REGISTER, u16>(this, regs.DE);
        break;
    case 0x14:
        INC<REGISTER, u8>(this, regs.D);
        break;
    case 0x15:
        DEC_<REGISTER, u8>(this, regs.D);
        break;
    case 0x16:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.D, read8(regs.PC++));
        break;
    case 0x17:
        RLA(this);
        break;
    case 0x18:
        JR(this);
        break;
    case 0x19:
        ADD_HLrr(this, regs.DE);
        break;
    case 0x1A:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(regs.DE));
        break;
    case 0x1B:
        DEC_<REGISTER, u16>(this, regs.DE);
        break;
    case 0x1C:
        INC<REGISTER, u8>(this, regs.E);
        break;
    case 0x1D:
        DEC_<REGISTER, u8>(this, regs.E);
        break;
    case 0x1E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.E, read8(regs.PC++));
        break;
    case 0x1F:
        RRA(this);
        break;
    case 0x20:
        instructionCycles += JR<JP_NZ>(this);
        break;
    case 0x21:
        LD<REGISTER, IMMEDIATE, u16>(this, regs.HL, read16(regs.PC));
        regs.PC += 2;
        break;
    case 0x22:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.A);
        regs.HL++;
        break;
    case 0x23:
        INC<REGISTER, u16>(this, regs.HL);
        break;
    case 0x24:
        INC<REGISTER, u8>(this, regs.H);
        break;
    case 0x25:
        DEC_<REGISTER, u8>(this, regs.H);
        break;
    case 0x26:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.H, read8(regs.PC++));
        break;
    case 0x27:
        DAA(this);
        break;
    case 0x28:
        instructionCycles += JR<JP_Z>(this);
        break;
    case 0x29:
        ADD_HLrr(this, regs.HL);
        break;
    case 0x2A:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(regs.HL++));
        break;
    case 0x2B:
        DEC_<REGISTER, u16>(this, regs.HL);
        break;
    case 0x2C:
        INC<REGISTER, u8>(this, regs.L);
        break;
    case 0x2D:
        DEC_<REGISTER, u8>(this, regs.L);
        break;
    case 0x2E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.L, read8(regs.PC++));
        break;
    case 0x2F:
        CPL(this);
        break;
    case 0x30:
        instructionCycles += JR<JP_NC>(this);
        break;
    case 0x31:
        LD<REGISTER, IMMEDIATE, u16>(this, regs.SP, read16(regs.PC));
        regs.PC += 2;
        break;
    case 0x32:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.A);
        regs.HL--;
        break;
    case 0x33:
        INC<REGISTER, u16>(this, regs.SP);
        break;
    case 0x34:
        INC<ADDRESS_PTR, u16>(this, regs.HL);
        break;
    case 0x35:
        DEC_<ADDRESS_PTR, u16>(this, regs.HL);
        break;
    case 0x36:
        LD<ADDRESS_PTR, IMMEDIATE, u16>(this, regs.HL, read8(regs.PC++));
        break;
    case 0x37:
        SCF(this);
        break;
    case 0x38:
        instructionCycles += JR<JP_C>(this);
        break;
    case 0x39:
        ADD_HLrr(this, regs.SP);
        break;
    case 0x3A:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(regs.HL--));
        break;
    case 0x3B:
        DEC_<REGISTER, u16>(this, regs.SP);
        break;
    case 0x3C:
        INC<REGISTER, u8>(this, regs.A);
        break;
    case 0x3D:
        DEC_<REGISTER, u8>(this, regs.A);
        break;
    case 0x3E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(regs.PC++));
        break;
    case 0x3F:
        CCF(this);
        break;
    case 0x40:
        LD<REGISTER, REGISTER, u8>(this, regs.B, regs.B);
        break;
    case 0x41:
        LD<REGISTER, REGISTER, u8>(this, regs.B, regs.C);
        break;
    case 0x42:
        LD<REGISTER, REGISTER, u8>(this, regs.B, regs.D);
        break;
    case 0x43:
        LD<REGISTER, REGISTER, u8>(this, regs.B, regs.E);
        break;
    case 0x44:
        LD<REGISTER, REGISTER, u8>(this, regs.B, regs.H);
        break;
    case 0x45:
        LD<REGISTER, REGISTER, u8>(this, regs.B, regs.L);
        break;
    case 0x46:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.B, read8(regs.HL));
        break;
    case 0x47:
        LD<REGISTER, REGISTER, u8>(this, regs.B, regs.A);
        break;
    case 0x48:
        LD<REGISTER, REGISTER, u8>(this, regs.C, regs.B);
        break;
    case 0x49:
        LD<REGISTER, REGISTER, u8>(this, regs.C, regs.C);
        break;
    case 0x4A:
        LD<REGISTER, REGISTER, u8>(this, regs.C, regs.D);
        break;
    case 0x4B:
        LD<REGISTER, REGISTER, u8>(this, regs.C, regs.E);
        break;
    case 0x4C:
        LD<REGISTER, REGISTER, u8>(this, regs.C, regs.H);
        break;
    case 0x4D:
        LD<REGISTER, REGISTER, u8>(this, regs.C, regs.L);
        break;
    case 0x4E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.C, read8(regs.HL));
        break;
    case 0x4F:
        LD<REGISTER, REGISTER, u8>(this, regs.C, regs.A);
        break;
    case 0x50:
        LD<REGISTER, REGISTER, u8>(this, regs.D, regs.B);
        break;
    case 0x51:
        LD<REGISTER, REGISTER, u8>(this, regs.D, regs.C);
        break;
    case 0x52:
        LD<REGISTER, REGISTER, u8>(this, regs.D, regs.D);
        break;
    case 0x53:
        LD<REGISTER, REGISTER, u8>(this, regs.D, regs.E);
        break;
    case 0x54:
        LD<REGISTER, REGISTER, u8>(this, regs.D, regs.H);
        break;
    case 0x55:
        LD<REGISTER, REGISTER, u8>(this, regs.D, regs.L);
        break;
    case 0x56:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.D, read8(regs.HL));
        break;
    case 0x57:
        LD<REGISTER, REGISTER, u8>(this, regs.D, regs.A);
        break;
    case 0x58:
        LD<REGISTER, REGISTER, u8>(this, regs.E, regs.B);
        break;
    case 0x59:
        LD<REGISTER, REGISTER, u8>(this, regs.E, regs.C);
        break;
    case 0x5A:
        LD<REGISTER, REGISTER, u8>(this, regs.E, regs.D);
        break;
    case 0x5B:
        LD<REGISTER, REGISTER, u8>(this, regs.E, regs.E);
        break;
    case 0x5C:
        LD<REGISTER, REGISTER, u8>(this, regs.E, regs.H);
        break;
    case 0x5D:
        LD<REGISTER, REGISTER, u8>(this, regs.E, regs.L);
        break;
    case 0x5E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.E, read8(regs.HL));
        break;
    case 0x5F:
        LD<REGISTER, REGISTER, u8>(this, regs.E, regs.A);
        break;
    case 0x60:
        LD<REGISTER, REGISTER, u8>(this, regs.H, regs.B);
        break;
    case 0x61:
        LD<REGISTER, REGISTER, u8>(this, regs.H, regs.C);
        break;
    case 0x62:
        LD<REGISTER, REGISTER, u8>(this, regs.H, regs.D);
        break;
    case 0x63:
        LD<REGISTER, REGISTER, u8>(this, regs.H, regs.E);
        break;
    case 0x64:
        LD<REGISTER, REGISTER, u8>(this, regs.H, regs.H);
        break;
    case 0x65:
        LD<REGISTER, REGISTER, u8>(this, regs.H, regs.L);
        break;
    case 0x66:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.H, read8(regs.HL));
        break;
    case 0x67:
        LD<REGISTER, REGISTER, u8>(this, regs.H, regs.A);
        break;
    case 0x68:
        LD<REGISTER, REGISTER, u8>(this, regs.L, regs.B);
        break;
    case 0x69:
        LD<REGISTER, REGISTER, u8>(this, regs.L, regs.C);
        break;
    case 0x6A:
        LD<REGISTER, REGISTER, u8>(this, regs.L, regs.D);
        break;
    case 0x6B:
        LD<REGISTER, REGISTER, u8>(this, regs.L, regs.E);
        break;
    case 0x6C:
        LD<REGISTER, REGISTER, u8>(this, regs.L, regs.H);
        break;
    case 0x6D:
        LD<REGISTER, REGISTER, u8>(this, regs.L, regs.L);
        break;
    case 0x6E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.L, read8(regs.HL));
        break;
    case 0x6F:
        LD<REGISTER, REGISTER, u8>(this, regs.L, regs.A);
        break;
    case 0x70:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.B);
        break;
    case 0x71:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.C);
        break;
    case 0x72:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.D);
        break;
    case 0x73:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.E);
        break;
    case 0x74:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.H);
        break;
    case 0x75:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.L);
        break;
    case 0x76:
        HALT(this);
        break;
    case 0x77:
        LD<ADDRESS_PTR, REGISTER, u16>(this, regs.HL, regs.A);
        break;
    case 0x78:
        LD<REGISTER, REGISTER, u8>(this, regs.A, regs.B);
        break;
    case 0x79:
        LD<REGISTER, REGISTER, u8>(this, regs.A, regs.C);
        break;
    case 0x7A:
        LD<REGISTER, REGISTER, u8>(this, regs.A, regs.D);
        break;
    case 0x7B:
        LD<REGISTER, REGISTER, u8>(this, regs.A, regs.E);
        break;
    case 0x7C:
        LD<REGISTER, REGISTER, u8>(this, regs.A, regs.H);
        break;
    case 0x7D:
        LD<REGISTER, REGISTER, u8>(this, regs.A, regs.L);
        break;
    case 0x7E:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(regs.HL));
        break;
    case 0x7F:
        LD<REGISTER, REGISTER, u8>(this, regs.A, regs.A);
        break;
    case 0x80:
        ADDC<REGISTER, u8>(this, regs.B, false);
        break;
    case 0x81:
        ADDC<REGISTER, u8>(this, regs.C, false);
        break;
    case 0x82:
        ADDC<REGISTER, u8>(this, regs.D, false);
        break;
    case 0x83:
        ADDC<REGISTER, u8>(this, regs.E, false);
        break;
    case 0x84:
        ADDC<REGISTER, u8>(this, regs.H, false);
        break;
    case 0x85:
        ADDC<REGISTER, u8>(this, regs.L, false);
        break;
    case 0x86:
        ADDC<IMMEDIATE, u8>(this, read8(regs.HL), false);
        break;
    case 0x87:
        ADDC<REGISTER, u8>(this, regs.A, false);
        break;
    case 0x88:
        ADDC<REGISTER, u8>(this, regs.B, true);
        break;
    case 0x89:
        ADDC<REGISTER, u8>(this, regs.C, true);
        break;
    case 0x8A:
        ADDC<REGISTER, u8>(this, regs.D, true);
        break;
    case 0x8B:
        ADDC<REGISTER, u8>(this, regs.E, true);
        break;
    case 0x8C:
        ADDC<REGISTER, u8>(this, regs.H, true);
        break;
    case 0x8D:
        ADDC<REGISTER, u8>(this, regs.L, true);
        break;
    case 0x8E:
        ADDC<IMMEDIATE, u8>(this, read8(regs.HL), true);
        break;
    case 0x8F:
        ADDC<REGISTER, u8>(this, regs.A, true);
        break;
    case 0x90:
        SUBC<REGISTER, u8>(this, regs.B, false);
        break;
    case 0x91:
        SUBC<REGISTER, u8>(this, regs.C, false);
        break;
    case 0x92:
        SUBC<REGISTER, u8>(this, regs.D, false);
        break;
    case 0x93:
        SUBC<REGISTER, u8>(this, regs.E, false);
        break;
    case 0x94:
        SUBC<REGISTER, u8>(this, regs.H, false);
        break;
    case 0x95:
        SUBC<REGISTER, u8>(this, regs.L, false);
        break;
    case 0x96:
        SUBC<IMMEDIATE, u8>(this, read8(regs.HL), false);
        break;
    case 0x97:
        SUBC<REGISTER, u8>(this, regs.A, false);
        break;
    case 0x98:
        SUBC<REGISTER, u8>(this, regs.B, true);
        break;
    case 0x99:
        SUBC<REGISTER, u8>(this, regs.C, true);
        break;
    case 0x9A:
        SUBC<REGISTER, u8>(this, regs.D, true);
        break;
    case 0x9B:
        SUBC<REGISTER, u8>(this, regs.E, true);
        break;
    case 0x9C:
        SUBC<REGISTER, u8>(this, regs.H, true);
        break;
    case 0x9D:
        SUBC<REGISTER, u8>(this, regs.L, true);
        break;
    case 0x9E:
        SUBC<IMMEDIATE, u8>(this, read8(regs.HL), true);
        break;
    case 0x9F:
        SUBC<REGISTER, u8>(this, regs.A, true);
        break;
    case 0xA0:
        BITWISE_OP<AND, REGISTER, u8>(this, regs.B);
        break;
    case 0xA1:
        BITWISE_OP<AND, REGISTER, u8>(this, regs.C);
        break;
    case 0xA2:
        BITWISE_OP<AND, REGISTER, u8>(this, regs.D);
        break;
    case 0xA3:
        BITWISE_OP<AND, REGISTER, u8>(this, regs.E);
        break;
    case 0xA4:
        BITWISE_OP<AND, REGISTER, u8>(this, regs.H);
        break;
    case 0xA5:
        BITWISE_OP<AND, REGISTER, u8>(this, regs.L);
        break;
    case 0xA6:
        BITWISE_OP<AND, ADDRESS_PTR, u16>(this, regs.HL);
        break;
    case 0xA7:
        BITWISE_OP<AND, REGISTER, u8>(this, regs.A);
        break;
    case 0xA8:
        BITWISE_OP<XOR, REGISTER, u8>(this, regs.B);
        break;
    case 0xA9:
        BITWISE_OP<XOR, REGISTER, u8>(this, regs.C);
        break;
    case 0xAA:
        BITWISE_OP<XOR, REGISTER, u8>(this, regs.D);
        break;
    case 0xAB:
        BITWISE_OP<XOR, REGISTER, u8>(this, regs.E);
        break;
    case 0xAC:
        BITWISE_OP<XOR, REGISTER, u8>(this, regs.H);
        break;
    case 0xAD:
        BITWISE_OP<XOR, REGISTER, u8>(this, regs.L);
        break;
    case 0xAE:
        BITWISE_OP<XOR, ADDRESS_PTR, u16>(this, regs.HL);
        break;
    case 0xAF:
        BITWISE_OP<XOR, REGISTER, u8>(this, regs.A);
        break;
    case 0xB0:
        BITWISE_OP<OR, REGISTER, u8>(this, regs.B);
        break;
    case 0xB1:
        BITWISE_OP<OR, REGISTER, u8>(this, regs.C);
        break;
    case 0xB2:
        BITWISE_OP<OR, REGISTER, u8>(this, regs.D);
        break;
    case 0xB3:
        BITWISE_OP<OR, REGISTER, u8>(this, regs.E);
        break;
    case 0xB4:
        BITWISE_OP<OR, REGISTER, u8>(this, regs.H);
        break;
    case 0xB5:
        BITWISE_OP<OR, REGISTER, u8>(this, regs.L);
        break;
    case 0xB6:
        BITWISE_OP<OR, ADDRESS_PTR, u16>(this, regs.HL);
        break;
    case 0xB7:
        BITWISE_OP<OR, REGISTER, u8>(this, regs.A);
        break;
    case 0xB8:
        CP<REGISTER, u8>(this, regs.B);
        break;
    case 0xB9:
        CP<REGISTER, u8>(this, regs.C);
        break;
    case 0xBA:
        CP<REGISTER, u8>(this, regs.D);
        break;
    case 0xBB:
        CP<REGISTER, u8>(this, regs.E);
        break;
    case 0xBC:
        CP<REGISTER, u8>(this, regs.H);
        break;
    case 0xBD:
        CP<REGISTER, u8>(this, regs.L);
        break;
    case 0xBE:
        CP<ADDRESS_PTR, u16>(this, regs.HL);
        break;
    case 0xBF:
        CP<REGISTER, u8>(this, regs.A);
        break;
    case 0xC0:
        instructionCycles += RET<JP_NZ>(this);
        break;
    case 0xC1:
        POP(this, regs.BC);
        break;
    case 0xC2:
        instructionCycles += JP<JP_NZ>(this);
        break;
    case 0xC3:
        JP(this);
        break;
    case 0xC4:
        instructionCycles += CALL<JP_NZ>(this);
        break;
    case 0xC5:
        PUSH(this, regs.BC);
        break;
    case 0xC6:
        ADDC<IMMEDIATE, u8>(this, read8(regs.PC++), false);
        break;
    case 0xC7:
        RST(this, 0x00);
        break;
    case 0xC8:
        instructionCycles += RET<JP_Z>(this);
        break;
    case 0xC9:
        RET(this);
        break;
    case 0xCA:
        instructionCycles += JP<JP_Z>(this);
        break;
    case 0xCB:
        {
            u8 cbOpcode = read8(regs.PC++);
            instructionCycles += extendedInstructionsCyclesTable[cbOpcode];
            decodeAndExecuteCBInstruction(cbOpcode);
        }
        break;
    case 0xCC:
        instructionCycles += CALL<JP_Z>(this);
        break;
    case 0xCD:
        CALL(this);
        break;
    case 0xCE:
        ADDC<IMMEDIATE, u8>(this, read8(regs.PC++), true);
        break;
    case 0xCF:
        RST(this, 0x08);
        break;
    case 0xD0:
        instructionCycles += RET<JP_NC>(this);
        break;
    case 0xD1:
        POP(this, regs.DE);
        break;
    case 0xD2:
        instructionCycles += JP<JP_NC>(this);
        break;
    case 0xD4:
        instructionCycles += CALL<JP_NC>(this);
        break;
    case 0xD5:
        PUSH(this, regs.DE);
        break;
    case 0xD6:
        SUBC<IMMEDIATE, u8>(this, read8(regs.PC++), false);
        break;
    case 0xD7:
        RST(this, 0x10);
        break;
    case 0xD8:
        instructionCycles += RET<JP_C>(this);
        break;
    case 0xD9:
        RETI(this);
        break;
    case 0xDA:
        instructionCycles += JP<JP_C>(this);
        break;
    case 0xDC:
        instructionCycles += CALL<JP_C>(this);
        break;
    case 0xDE:
        SUBC<IMMEDIATE, u8>(this, read8(regs.PC++), true);
        break;
    case 0xDF:
        RST(this, 0x18);
        break;
    case 0xE0:
        {
            u16 address = 0xFF00 | read8(regs.PC++);
            LD<ADDRESS_PTR, REGISTER, u16>(this, address, regs.A);
        }
        break;
    case 0xE1:
        POP(this, regs.HL);
        break;
    case 0xE2:
        {
            u16 address = 0xFF00 | regs.C;
            LD<ADDRESS_PTR, REGISTER, u16>(this, address, regs.A);
        }
        break;
    case 0xE5:
        PUSH(this, regs.HL);
        break;
    case 0xE6:
        BITWISE_OP<AND, IMMEDIATE, u8>(this, read8(regs.PC++));
        break;
    case 0xE7:
        RST(this, 0x20);
        break;
    case 0xE8:
        ADD_SPi8(this, static_cast<s8>(read8(regs.PC++)));
        break;
    case 0xE9:
        JP(this, true); // regs.HL
        break;
    case 0xEA:
        {
            u16 address = read16(regs.PC);
            LD<ADDRESS_PTR, REGISTER, u16>(this, address, regs.A);
            regs.PC += 2;
        }
        break;
    case 0xEE:
        BITWISE_OP<XOR, IMMEDIATE, u8>(this, read8(regs.PC++));
        break;
    case 0xEF:
        RST(this, 0x28);
        break;
    case 0xF0:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(0xFF00 | read8(regs.PC++)));
        break;
    case 0xF1:
        POP(this, regs.AF);
        regs.AF &= 0xFFF0;
        break;
    case 0xF2:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(0xFF00 | regs.C));
        break;
    case 0xF3:
        DI(system);
        break;
    case 0xF5:
        regs.AF &= 0xFFF0;
        PUSH(this, regs.AF);
        break;
    case 0xF6:
        BITWISE_OP<OR, IMMEDIATE, u8>(this, read8(regs.PC++));
        break;
    case 0xF7:
        RST(this, 0x30);
        break;
    case 0xF8:
        LD_HLSPi8(this, static_cast<s8>(read8(regs.PC++)));
        break;
    case 0xF9:
        LD<REGISTER, REGISTER, u16>(this, regs.SP, regs.HL);
        break;
    case 0xFA:
        LD<REGISTER, IMMEDIATE, u8>(this, regs.A, read8(read16(regs.PC)));
        regs.PC += 2;
        break;
    case 0xFB:
        EI(system);
        break;
    case 0xFE:
        CP<IMMEDIATE, u8>(this, read8(regs.PC++));
        break;
    case 0xFF:
        RST(this, 0x38);
        break;
    default:
        printf("\nOpcode %02X not implemented.\n", opcode);
        break;
    }
}

auto gb::SM83CPU::decodeAndExecuteCBInstruction(u8 cbOpcode) -> void
{
    switch (cbOpcode)
    {
    case 0x00:
        RLC<u8>(this, regs.B);
        break;
    case 0x01:
        RLC<u8>(this, regs.C);
        break;
    case 0x02:
        RLC<u8>(this, regs.D);
        break;
    case 0x03:
        RLC<u8>(this, regs.E);
        break;
    case 0x04:
        RLC<u8>(this, regs.H);
        break;
    case 0x05:
        RLC<u8>(this, regs.L);
        break;
    case 0x06:
        RLC<u16>(this, regs.HL);
        break;
    case 0x07:
        RLC<u8>(this, regs.A);
        break;
    case 0x08:
        RRC<u8>(this, regs.B);
        break;
    case 0x09:
        RRC<u8>(this, regs.C);
        break;
    case 0x0A:
        RRC<u8>(this, regs.D);
        break;
    case 0x0B:
        RRC<u8>(this, regs.E);
        break;
    case 0x0C:
        RRC<u8>(this, regs.H);
        break;
    case 0x0D:
        RRC<u8>(this, regs.L);
        break;
    case 0x0E:
        RRC<u16>(this, regs.HL);
        break;
    case 0x0F:
        RRC<u8>(this, regs.A);
        break;
    case 0x10:
        RL<u8>(this, regs.B);
        break;
    case 0x11:
        RL<u8>(this, regs.C);
        break;
    case 0x12:
        RL<u8>(this, regs.D);
        break;
    case 0x13:
        RL<u8>(this, regs.E);
        break;
    case 0x14:
        RL<u8>(this, regs.H);
        break;
    case 0x15:
        RL<u8>(this, regs.L);
        break;
    case 0x16:
        RL<u16>(this, regs.HL);
        break;
    case 0x17:
        RL<u8>(this, regs.A);
        break;
    case 0x18:
        RR<u8>(this, regs.B);
        break;
    case 0x19:
        RR<u8>(this, regs.C);
        break;
    case 0x1A:
        RR<u8>(this, regs.D);
        break;
    case 0x1B:
        RR<u8>(this, regs.E);
        break;
    case 0x1C:
        RR<u8>(this, regs.H);
        break;
    case 0x1D:
        RR<u8>(this, regs.L);
        break;
    case 0x1E:
        RR<u16>(this, regs.HL);
        break;
    case 0x1F:
        RR<u8>(this, regs.A);
        break;
    case 0x20:
        SLA<u8>(this, regs.B);
        break;
    case 0x21:
        SLA<u8>(this, regs.C);
        break;
    case 0x22:
        SLA<u8>(this, regs.D);
        break;
    case 0x23:
        SLA<u8>(this, regs.E);
        break;
    case 0x24:
        SLA<u8>(this, regs.H);
        break;
    case 0x25:
        SLA<u8>(this, regs.L);
        break;
    case 0x26:
        SLA<u16>(this, regs.HL);
        break;
    case 0x27:
        SLA<u8>(this, regs.A);
        break;
    case 0x28:
        SRA<u8>(this, regs.B);
        break;
    case 0x29:
        SRA<u8>(this, regs.C);
        break;
    case 0x2A:
        SRA<u8>(this, regs.D);
        break;
    case 0x2B:
        SRA<u8>(this, regs.E);
        break;
    case 0x2C:
        SRA<u8>(this, regs.H);
        break;
    case 0x2D:
        SRA<u8>(this, regs.L);
        break;
    case 0x2E:
        SRA<u16>(this, regs.HL);
        break;
    case 0x2F:
        SRA<u8>(this, regs.A);
        break;
    case 0x30:
        SWAP<u8>(this, regs.B);
        break;
    case 0x31:
        SWAP<u8>(this, regs.C);
        break;
    case 0x32:
        SWAP<u8>(this, regs.D);
        break;
    case 0x33:
        SWAP<u8>(this, regs.E);
        break;
    case 0x34:
        SWAP<u8>(this, regs.H);
        break;
    case 0x35:
        SWAP<u8>(this, regs.L);
        break;
    case 0x36:
        SWAP<u16>(this, regs.HL);
        break;
    case 0x37:
        SWAP<u8>(this, regs.A);
        break;
    case 0x38:
        SRL<u8>(this, regs.B);
        break;
    case 0x39:
        SRL<u8>(this, regs.C);
        break;
    case 0x3A:
        SRL<u8>(this, regs.D);
        break;
    case 0x3B:
        SRL<u8>(this, regs.E);
        break;
    case 0x3C:
        SRL<u8>(this, regs.H);
        break;
    case 0x3D:
        SRL<u8>(this, regs.L);
        break;
    case 0x3E:
        SRL<u16>(this, regs.HL);
        break;
    case 0x3F:
        SRL<u8>(this, regs.A);
        break;
    case 0x40:
        BIT_<0, u8>(this, regs.B);
        break;
    case 0x41:
        BIT_<0, u8>(this, regs.C);
        break;
    case 0x42:
        BIT_<0, u8>(this, regs.D);
        break;
    case 0x43:
        BIT_<0, u8>(this, regs.E);
        break;
    case 0x44:
        BIT_<0, u8>(this, regs.H);
        break;
    case 0x45:
        BIT_<0, u8>(this, regs.L);
        break;
    case 0x46:
        BIT_<0, u16>(this, regs.HL);
        break;
    case 0x47:
        BIT_<0, u8>(this, regs.A);
        break;
    case 0x48:
        BIT_<1, u8>(this, regs.B);
        break;
    case 0x49:
        BIT_<1, u8>(this, regs.C);
        break;
    case 0x4A:
        BIT_<1, u8>(this, regs.D);
        break;
    case 0x4B:
        BIT_<1, u8>(this, regs.E);
        break;
    case 0x4C:
        BIT_<1, u8>(this, regs.H);
        break;
    case 0x4D:
        BIT_<1, u8>(this, regs.L);
        break;
    case 0x4E:
        BIT_<1, u16>(this, regs.HL);
        break;
    case 0x4F:
        BIT_<1, u8>(this, regs.A);
        break;
    case 0x50:
        BIT_<2, u8>(this, regs.B);
        break;
    case 0x51:
        BIT_<2, u8>(this, regs.C);
        break;
    case 0x52:
        BIT_<2, u8>(this, regs.D);
        break;
    case 0x53:
        BIT_<2, u8>(this, regs.E);
        break;
    case 0x54:
        BIT_<2, u8>(this, regs.H);
        break;
    case 0x55:
        BIT_<2, u8>(this, regs.L);
        break;
    case 0x56:
        BIT_<2, u16>(this, regs.HL);
        break;
    case 0x57:
        BIT_<2, u8>(this, regs.A);
        break;
    case 0x58:
        BIT_<3, u8>(this, regs.B);
        break;
    case 0x59:
        BIT_<3, u8>(this, regs.C);
        break;
    case 0x5A:
        BIT_<3, u8>(this, regs.D);
        break;
    case 0x5B:
        BIT_<3, u8>(this, regs.E);
        break;
    case 0x5C:
        BIT_<3, u8>(this, regs.H);
        break;
    case 0x5D:
        BIT_<3, u8>(this, regs.L);
        break;
    case 0x5E:
        BIT_<3, u16>(this, regs.HL);
        break;
    case 0x5F:
        BIT_<3, u8>(this, regs.A);
        break;
    case 0x60:
        BIT_<4, u8>(this, regs.B);
        break;
    case 0x61:
        BIT_<4, u8>(this, regs.C);
        break;
    case 0x62:
        BIT_<4, u8>(this, regs.D);
        break;
    case 0x63:
        BIT_<4, u8>(this, regs.E);
        break;
    case 0x64:
        BIT_<4, u8>(this, regs.H);
        break;
    case 0x65:
        BIT_<4, u8>(this, regs.L);
        break;
    case 0x66:
        BIT_<4, u16>(this, regs.HL);
        break;
    case 0x67:
        BIT_<4, u8>(this, regs.A);
        break;
    case 0x68:
        BIT_<5, u8>(this, regs.B);
        break;
    case 0x69:
        BIT_<5, u8>(this, regs.C);
        break;
    case 0x6A:
        BIT_<5, u8>(this, regs.D);
        break;
    case 0x6B:
        BIT_<5, u8>(this, regs.E);
        break;
    case 0x6C:
        BIT_<5, u8>(this, regs.H);
        break;
    case 0x6D:
        BIT_<5, u8>(this, regs.L);
        break;
    case 0x6E:
        BIT_<5, u16>(this, regs.HL);
        break;
    case 0x6F:
        BIT_<5, u8>(this, regs.A);
        break;
    case 0x70:
        BIT_<6, u8>(this, regs.B);
        break;
    case 0x71:
        BIT_<6, u8>(this, regs.C);
        break;
    case 0x72:
        BIT_<6, u8>(this, regs.D);
        break;
    case 0x73:
        BIT_<6, u8>(this, regs.E);
        break;
    case 0x74:
        BIT_<6, u8>(this, regs.H);
        break;
    case 0x75:
        BIT_<6, u8>(this, regs.L);
        break;
    case 0x76:
        BIT_<6, u16>(this, regs.HL);
        break;
    case 0x77:
        BIT_<6, u8>(this, regs.A);
        break;
    case 0x78:
        BIT_<7, u8>(this, regs.B);
        break;
    case 0x79:
        BIT_<7, u8>(this, regs.C);
        break;
    case 0x7A:
        BIT_<7, u8>(this, regs.D);
        break;
    case 0x7B:
        BIT_<7, u8>(this, regs.E);
        break;
    case 0x7C:
        BIT_<7, u8>(this, regs.H);
        break;
    case 0x7D:
        BIT_<7, u8>(this, regs.L);
        break;
    case 0x7E:
        BIT_<7, u16>(this, regs.HL);
        break;
    case 0x7F:
        BIT_<7, u8>(this, regs.A);
        break;
    case 0x80:
        RES<0, u8>(this, regs.B);
        break;
    case 0x81:
        RES<0, u8>(this, regs.C);
        break;
    case 0x82:
        RES<0, u8>(this, regs.D);
        break;
    case 0x83:
        RES<0, u8>(this, regs.E);
        break;
    case 0x84:
        RES<0, u8>(this, regs.H);
        break;
    case 0x85:
        RES<0, u8>(this, regs.L);
        break;
    case 0x86:
        RES<0, u16>(this, regs.HL);
        break;
    case 0x87:
        RES<0, u8>(this, regs.A);
        break;
    case 0x88:
        RES<1, u8>(this, regs.B);
        break;
    case 0x89:
        RES<1, u8>(this, regs.C);
        break;
    case 0x8A:
        RES<1, u8>(this, regs.D);
        break;
    case 0x8B:
        RES<1, u8>(this, regs.E);
        break;
    case 0x8C:
        RES<1, u8>(this, regs.H);
        break;
    case 0x8D:
        RES<1, u8>(this, regs.L);
        break;
    case 0x8E:
        RES<1, u16>(this, regs.HL);
        break;
    case 0x8F:
        RES<1, u8>(this, regs.A);
        break;
    case 0x90:
        RES<2, u8>(this, regs.B);
        break;
    case 0x91:
        RES<2, u8>(this, regs.C);
        break;
    case 0x92:
        RES<2, u8>(this, regs.D);
        break;
    case 0x93:
        RES<2, u8>(this, regs.E);
        break;
    case 0x94:
        RES<2, u8>(this, regs.H);
        break;
    case 0x95:
        RES<2, u8>(this, regs.L);
        break;
    case 0x96:
        RES<2, u16>(this, regs.HL);
        break;
    case 0x97:
        RES<2, u8>(this, regs.A);
        break;
    case 0x98:
        RES<3, u8>(this, regs.B);
        break;
    case 0x99:
        RES<3, u8>(this, regs.C);
        break;
    case 0x9A:
        RES<3, u8>(this, regs.D);
        break;
    case 0x9B:
        RES<3, u8>(this, regs.E);
        break;
    case 0x9C:
        RES<3, u8>(this, regs.H);
        break;
    case 0x9D:
        RES<3, u8>(this, regs.L);
        break;
    case 0x9E:
        RES<3, u16>(this, regs.HL);
        break;
    case 0x9F:
        RES<3, u8>(this, regs.A);
        break;
    case 0xA0:
        RES<4, u8>(this, regs.B);
        break;
    case 0xA1:
        RES<4, u8>(this, regs.C);
        break;
    case 0xA2:
        RES<4, u8>(this, regs.D);
        break;
    case 0xA3:
        RES<4, u8>(this, regs.E);
        break;
    case 0xA4:
        RES<4, u8>(this, regs.H);
        break;
    case 0xA5:
        RES<4, u8>(this, regs.L);
        break;
    case 0xA6:
        RES<4, u16>(this, regs.HL);
        break;
    case 0xA7:
        RES<4, u8>(this, regs.A);
        break;
    case 0xA8:
        RES<5, u8>(this, regs.B);
        break;
    case 0xA9:
        RES<5, u8>(this, regs.C);
        break;
    case 0xAA:
        RES<5, u8>(this, regs.D);
        break;
    case 0xAB:
        RES<5, u8>(this, regs.E);
        break;
    case 0xAC:
        RES<5, u8>(this, regs.H);
        break;
    case 0xAD:
        RES<5, u8>(this, regs.L);
        break;
    case 0xAE:
        RES<5, u16>(this, regs.HL);
        break;
    case 0xAF:
        RES<5, u8>(this, regs.A);
        break;
    case 0xB0:
        RES<6, u8>(this, regs.B);
        break;
    case 0xB1:
        RES<6, u8>(this, regs.C);
        break;
    case 0xB2:
        RES<6, u8>(this, regs.D);
        break;
    case 0xB3:
        RES<6, u8>(this, regs.E);
        break;
    case 0xB4:
        RES<6, u8>(this, regs.H);
        break;
    case 0xB5:
        RES<6, u8>(this, regs.L);
        break;
    case 0xB6:
        RES<6, u16>(this, regs.HL);
        break;
    case 0xB7:
        RES<6, u8>(this, regs.A);
        break;
    case 0xB8:
        RES<7, u8>(this, regs.B);
        break;
    case 0xB9:
        RES<7, u8>(this, regs.C);
        break;
    case 0xBA:
        RES<7, u8>(this, regs.D);
        break;
    case 0xBB:
        RES<7, u8>(this, regs.E);
        break;
    case 0xBC:
        RES<7, u8>(this, regs.H);
        break;
    case 0xBD:
        RES<7, u8>(this, regs.L);
        break;
    case 0xBE:
        RES<7, u16>(this, regs.HL);
        break;
    case 0xBF:
        RES<7, u8>(this, regs.A);
        break;
    case 0xC0:
        SET<0, u8>(this, regs.B);
        break;
    case 0xC1:
        SET<0, u8>(this, regs.C);
        break;
    case 0xC2:
        SET<0, u8>(this, regs.D);
        break;
    case 0xC3:
        SET<0, u8>(this, regs.E);
        break;
    case 0xC4:
        SET<0, u8>(this, regs.H);
        break;
    case 0xC5:
        SET<0, u8>(this, regs.L);
        break;
    case 0xC6:
        SET<0, u16>(this, regs.HL);
        break;
    case 0xC7:
        SET<0, u8>(this, regs.A);
        break;
    case 0xC8:
        SET<1, u8>(this, regs.B);
        break;
    case 0xC9:
        SET<1, u8>(this, regs.C);
        break;
    case 0xCA:
        SET<1, u8>(this, regs.D);
        break;
    case 0xCB:
        SET<1, u8>(this, regs.E);
        break;
    case 0xCC:
        SET<1, u8>(this, regs.H);
        break;
    case 0xCD:
        SET<1, u8>(this, regs.L);
        break;
    case 0xCE:
        SET<1, u16>(this, regs.HL);
        break;
    case 0xCF:
        SET<1, u8>(this, regs.A);
        break;
    case 0xD0:
        SET<2, u8>(this, regs.B);
        break;
    case 0xD1:
        SET<2, u8>(this, regs.C);
        break;
    case 0xD2:
        SET<2, u8>(this, regs.D);
        break;
    case 0xD3:
        SET<2, u8>(this, regs.E);
        break;
    case 0xD4:
        SET<2, u8>(this, regs.H);
        break;
    case 0xD5:
        SET<2, u8>(this, regs.L);
        break;
    case 0xD6:
        SET<2, u16>(this, regs.HL);
        break;
    case 0xD7:
        SET<2, u8>(this, regs.A);
        break;
    case 0xD8:
        SET<3, u8>(this, regs.B);
        break;
    case 0xD9:
        SET<3, u8>(this, regs.C);
        break;
    case 0xDA:
        SET<3, u8>(this, regs.D);
        break;
    case 0xDB:
        SET<3, u8>(this, regs.E);
        break;
    case 0xDC:
        SET<3, u8>(this, regs.H);
        break;
    case 0xDD:
        SET<3, u8>(this, regs.L);
        break;
    case 0xDE:
        SET<3, u16>(this, regs.HL);
        break;
    case 0xDF:
        SET<3, u8>(this, regs.A);
        break;
    case 0xE0:
        SET<4, u8>(this, regs.B);
        break;
    case 0xE1:
        SET<4, u8>(this, regs.C);
        break;
    case 0xE2:
        SET<4, u8>(this, regs.D);
        break;
    case 0xE3:
        SET<4, u8>(this, regs.E);
        break;
    case 0xE4:
        SET<4, u8>(this, regs.H);
        break;
    case 0xE5:
        SET<4, u8>(this, regs.L);
        break;
    case 0xE6:
        SET<4, u16>(this, regs.HL);
        break;
    case 0xE7:
        SET<4, u8>(this, regs.A);
        break;
    case 0xE8:
        SET<5, u8>(this, regs.B);
        break;
    case 0xE9:
        SET<5, u8>(this, regs.C);
        break;
    case 0xEA:
        SET<5, u8>(this, regs.D);
        break;
    case 0xEB:
        SET<5, u8>(this, regs.E);
        break;
    case 0xEC:
        SET<5, u8>(this, regs.H);
        break;
    case 0xED:
        SET<5, u8>(this, regs.L);
        break;
    case 0xEE:
        SET<5, u16>(this, regs.HL);
        break;
    case 0xEF:
        SET<5, u8>(this, regs.A);
        break;
    case 0xF0:
        SET<6, u8>(this, regs.B);
        break;
    case 0xF1:
        SET<6, u8>(this, regs.C);
        break;
    case 0xF2:
        SET<6, u8>(this, regs.D);
        break;
    case 0xF3:
        SET<6, u8>(this, regs.E);
        break;
    case 0xF4:
        SET<6, u8>(this, regs.H);
        break;
    case 0xF5:
        SET<6, u8>(this, regs.L);
        break;
    case 0xF6:
        SET<6, u16>(this, regs.HL);
        break;
    case 0xF7:
        SET<6, u8>(this, regs.A);
        break;
    case 0xF8:
        SET<7, u8>(this, regs.B);
        break;
    case 0xF9:
        SET<7, u8>(this, regs.C);
        break;
    case 0xFA:
        SET<7, u8>(this, regs.D);
        break;
    case 0xFB:
        SET<7, u8>(this, regs.E);
        break;
    case 0xFC:
        SET<7, u8>(this, regs.H);
        break;
    case 0xFD:
        SET<7, u8>(this, regs.L);
        break;
    case 0xFE:
        SET<7, u16>(this, regs.HL);
        break;
    case 0xFF:
        SET<7, u8>(this, regs.A);
        break;
    default:
        printf("\n0xCB extended opcode %02X not implemented.\n", cbOpcode);
        break;
    }
}

auto gb::SM83CPU::setRegisterValuesPostBootROM() -> void
{
    regs.AF = (read8(0x014D) == 0x00) ? 0x0100 : 0x01B0;
    regs.BC = 0x0013;
    regs.DE = 0x00D8;
    regs.HL = 0x014D;
    regs.SP = 0xFFFE;
    regs.PC = 0x1000;

    write8(0xFF00, 0xCF);
    write8(0xFF01, 0x00);
    write8(0xFF02, 0x7E);
    write8(0xFF04, 0xAB);
    write8(0xFF05, 0x00);
    write8(0xFF06, 0x00);
    write8(0xFF07, 0xF8);
    write8(0xFF0F, 0xE1);
    write8(0xFF10, 0x80);
    write8(0xFF11, 0xBF);
    write8(0xFF12, 0xF3);
    write8(0xFF13, 0xFF);
    write8(0xFF14, 0xBF);
    write8(0xFF16, 0x3F);
    write8(0xFF17, 0x00);
    write8(0xFF18, 0xFF);
    write8(0xFF19, 0xBF);
    write8(0xFF1A, 0x7F);
    write8(0xFF1B, 0xFF);
    write8(0xFF1C, 0x9F);
    write8(0xFF1D, 0xFF);
    write8(0xFF1E, 0xBF);
    write8(0xFF20, 0xFF);
    write8(0xFF21, 0x00);
    write8(0xFF22, 0x00);
    write8(0xFF23, 0xBF);
    write8(0xFF24, 0x77);
    write8(0xFF25, 0xF3);
    write8(0xFF26, 0xF1);
    write8(0xFF40, 0x91);
    write8(0xFF41, 0x85);
    write8(0xFF42, 0x00);
    write8(0xFF43, 0x00);
    write8(0xFF44, 0x00);
    write8(0xFF45, 0x00);
    write8(0xFF46, 0xFF);
    write8(0xFF47, 0xFC);
    /*write8(0xFF48, 0x85);
    write8(0xFF49, 0x85);*/
    write8(0xFF4A, 0x00);
    write8(0xFF4B, 0x00);
    write8(0xFF4C, 0xFF);
    write8(0xFF4D, 0xFF);
    write8(0xFF4F, 0xFF);
    write8(0xFF51, 0xFF);
    write8(0xFF52, 0xFF);
    write8(0xFF53, 0xFF);
    write8(0xFF54, 0xFF);
    write8(0xFF55, 0xFF);
    write8(0xFF56, 0xFF);
    write8(0xFF68, 0xFF);
    write8(0xFF69, 0xFF);
    write8(0xFF6A, 0xFF);
    write8(0xFF6B, 0xFF);
    write8(0xFF70, 0xFF);
    write8(0xFFFF, 0x00);
}
