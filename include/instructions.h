/*
 * Copyright (C) 2023 pabletefest
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once
#include "cpu_sm83.h"
#include "gb.h"

#include <cassert>

namespace gb
{
    enum OperandsType
    {
        REGISTER,
        ADDRESS_PTR,
        IMMEDIATE
    };

    enum BitwiseOperation
    {
        AND,
        XOR,
        OR
    };

    enum JumpConditionFlags
    {
        JP_NZ,
        JP_Z,
        JP_NC,
        JP_C
    };

    static constexpr u8 instructionsCyclesTable[256] = {
        4, 12, 8, 8, 4, 4, 8, 4, 20, 8, 8, 8, 4, 4, 8, 4,
        4, 12, 8, 8, 4, 4, 8, 4, 12, 8, 8, 8, 4, 4, 8, 4,
        8, 12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,
        8, 12, 8, 8, 12, 12, 12, 4, 8, 8, 8, 8, 4, 4, 8, 4,
        4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
        4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
        4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
        8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,
        4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
        4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
        4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
        4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
        8, 12, 12, 16, 12, 16, 8, 16, 8, 16, 12, 0/*4*/, 12, 24, 8, 16,
        8, 12, 12, 0, 12, 16, 8, 16, 8, 16, 12, 0, 12, 0, 8, 16,
        12, 12, 8, 0, 0, 16, 8, 16, 16, 4, 16, 0, 0, 0, 8, 16,
        12, 12, 8, 4, 0, 16, 8, 16, 12, 8, 16, 4, 0, 0, 8, 16,
    };

    static constexpr u8 extendedInstructionsCyclesTable[256] = {
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    };

    template<OperandsType DST_TYPE, OperandsType SRC_TYPE, typename Operand>
    static constexpr auto LD(gb::SM83CPU* cpu, Operand& dst, const Operand& src) -> void
    {
        u16 value = 0;

        if constexpr ((SRC_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>) || SRC_TYPE == IMMEDIATE)
            value = (std::is_same_v<Operand, u16>) ? src : static_cast<u8>(src) & 0x00FF;
        else if constexpr (SRC_TYPE == ADDRESS_PTR)
            value = /*(std::is_same_v<Operand, u16>) ? cpu->read16(src) : */cpu->read8(src) & 0x00FF;
        else
            assert(false && "Error in LD opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary variable");

        if constexpr ((DST_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>) || DST_TYPE == IMMEDIATE)
            dst = (std::is_same_v<Operand, u16>) ? value : static_cast<u8>(value) & 0x00FF;
        else if constexpr (DST_TYPE == ADDRESS_PTR)
        {
            /*(std::is_same_v<Operand, u16>) ? cpu->write16(dst, value) : cpu->write8(dst, static_cast<u8>(value));*/
            /*if (std::is_same_v<Operand, u16> && SRC_TYPE == REGISTER)
                cpu->write16(dst, value);
            else
                cpu->write8(dst, static_cast<u8>(value));*/

            cpu->write8(dst, static_cast<u8>(value));
        }   
        else
            assert(false && "Error in LD opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary variable");
    }

    static auto LD_u16SP(gb::SM83CPU* cpu, const u16& address) -> void
    {
        cpu->write16(address, cpu->regs.SP);
    }

    static auto PUSH(gb::SM83CPU* cpu, const u16& src) -> void
    {
        // cpu->SP -= 2;
        // cpu->write16(cpu->SP, src);

        cpu->write8(--cpu->regs.SP, (src >> 8) & 0x00FF);
        cpu->write8(--cpu->regs.SP, src & 0x00FF);
    }

    static auto POP(gb::SM83CPU* cpu, u16& dst) -> void
    {
        // cpu->read16(cpu->SP, dst);
        // cpu->SP += 2;

        dst = cpu->read8(cpu->regs.SP++) & 0x00FF;
        dst |= (static_cast<u16>(cpu->read8(cpu->regs.SP++)) << 8) & 0xFF00;
    }

    template<OperandsType SRC_TYPE, typename Operand>
    static constexpr auto ADDC(gb::SM83CPU* cpu, const Operand& src, bool addCarryBit = false) -> void
    {
        u16 result, operand = 0;

        if constexpr ((SRC_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>) || SRC_TYPE == IMMEDIATE)
            result = (std::is_same_v<Operand, u16>) ? src : static_cast<u8>(src) & 0x00FF;
        else if constexpr (SRC_TYPE == ADDRESS_PTR)
            result = (std::is_same_v<Operand, u16>) ? cpu->read16(src) : cpu->read8(src) & 0x00FF;
        else
            assert(false && "Error in ADD/ADC opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary variable");
        
        u16 originalValue = cpu->regs.A;
        u16 originalOperand = result;
        operand = result + ((addCarryBit) ? cpu->regs.flags.C : 0);
        result = cpu->regs.A + operand;
        cpu->regs.A = result & 0x00FF;

        cpu->setFlag(gb::Z, cpu->regs.A == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, ((originalValue & 0x0F) + (originalOperand & 0x0F) + ((addCarryBit) ? cpu->regs.flags.C : 0)) > 0x0F);
        cpu->setFlag(gb::C, result > 0xFF);
    }

    static auto ADD_SPi8(gb::SM83CPU* cpu, const s8& immediate) -> void
    {
        u16 reg = cpu->regs.SP;
        cpu->regs.SP += immediate;

        //u16 result = cpu->regs.SP;
        //s16 operand = immediate;

        cpu->setFlag(gb::Z, 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, ((reg & 0x0F) + (immediate & 0x0F)) > 0x0F);
        cpu->setFlag(gb::C, ((reg & 0xFF) + (immediate & 0xFF)) > 0xFF);
    }

    static auto LD_HLSPi8(gb::SM83CPU* cpu, const s8& immediate) -> void
    {
        u16 reg = cpu->regs.SP;
        cpu->regs.HL = cpu->regs.SP + immediate;

        cpu->setFlag(gb::Z, 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, ((reg & 0x0F) + (immediate & 0x0F)) > 0x0F);
        cpu->setFlag(gb::C, ((reg & 0xFF) + (immediate & 0xFF)) > 0xFF);
    }

    static auto ADD_HLrr(gb::SM83CPU* cpu, const u16& reg) -> void
    {
        u16 operand = reg;
        cpu->regs.HL += reg;

        u16 result = cpu->regs.HL;

        auto test = ((operand & 0x0FFF) + ((result - operand) & 0x0FFF)) > 0x0FFF;

        //cpu->setFlag(gb::Z, 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, ((operand & 0x0FFF) + ((result - operand) & 0x0FFF)) > 0x0FFF);
        cpu->setFlag(gb::C, ((operand & 0xFFFF) + ((result - operand) & 0xFFFF)) > 0xFFFF);
    }

    template<OperandsType SRC_TYPE, typename Operand>
    static constexpr auto SUBC(gb::SM83CPU* cpu, const Operand& src, bool subCarryBit = false) -> void
    {
        u16 operand = 0;
        u16 result = 0;

        if constexpr ((SRC_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>) || SRC_TYPE == IMMEDIATE)
            operand = (std::is_same_v<Operand, u16>) ? src : static_cast<u8>(src) & 0x00FF;
        else if constexpr (SRC_TYPE == ADDRESS_PTR)
            operand = (std::is_same_v<Operand, u16>) ? cpu->read16(src) : cpu->read8(src) & 0x00FF;
        else
            assert(false && "Error in SUB/SBD opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary variable");

        //operand -= ((subCarryBit) ? cpu->regs.flags.C : 0);

        result = cpu->regs.A - operand - ((subCarryBit) ? cpu->regs.flags.C : 0);
        cpu->regs.A = result & 0x00FF;

        cpu->setFlag(gb::Z, cpu->regs.A == 0);
        cpu->setFlag(gb::N, 1);
        cpu->setFlag(gb::H, static_cast<s16>((((result + operand + ((subCarryBit) ? cpu->regs.flags.C : 0)) & 0x0F) - (operand & 0x0F) - ((subCarryBit) ? cpu->regs.flags.C : 0))) < 0);
        cpu->setFlag(gb::C, result > 0xFF);
    }

    template<BitwiseOperation OP_TYPE, OperandsType SRC_TYPE, typename Operand>
    static constexpr auto BITWISE_OP(gb::SM83CPU* cpu, const Operand& src) -> void
    {
        u16 operand = 0;
        u16 result = 0;

        if constexpr ((SRC_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>) || SRC_TYPE == IMMEDIATE)
            operand = (std::is_same_v<Operand, u16>) ? src : static_cast<u8>(src) & 0x00FF;
        else if constexpr (SRC_TYPE == ADDRESS_PTR)
            operand = /*(std::is_same_v<Operand, u16>) ? cpu->read16(src) : */cpu->read8(src) & 0x00FF;
        else
            assert(false && "Error in AND/XOR/OR opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary variable");

        if constexpr (OP_TYPE == AND)
            result = cpu->regs.A & operand;
        else if constexpr (OP_TYPE == XOR)
            result = cpu->regs.A ^ operand;
        else if constexpr (OP_TYPE == OR)
            result = cpu->regs.A | operand;
        else
            assert(false && "Error in AND/XOR/OR opcode: specify a valid bitwise operation!");

        cpu->regs.A = static_cast<u8>(result);

        cpu->setFlag(gb::Z, result == 0);
        cpu->setFlag(gb::N, 0);

        if constexpr (OP_TYPE == AND)
            cpu->setFlag(gb::H, 1);
        else
            cpu->setFlag(gb::H, 0);

        cpu->setFlag(gb::C, 0);
    }

    template<OperandsType SRC_TYPE, typename Operand>
    static constexpr auto CP(gb::SM83CPU* cpu, const Operand& src) -> void
    {
        u16 operand = 0;

        if constexpr ((SRC_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>) || SRC_TYPE == IMMEDIATE)
            operand = (std::is_same_v<Operand, u16>) ? src : static_cast<u8>(src) & 0x00FF;
        else if constexpr (SRC_TYPE == ADDRESS_PTR)
            operand = /*(std::is_same_v<Operand, u16>) ? cpu->read16(src) :*/ cpu->read8(src) & 0x00FF;
        else
            assert(false && "Error in CP opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary variable");

        u16 result = cpu->regs.A - operand;

        cpu->setFlag(gb::Z, result == 0);
        cpu->setFlag(gb::N, 1);
        cpu->setFlag(gb::H, (((result + operand) & 0x0F) - (operand & 0x0F)) < 0);
        cpu->setFlag(gb::C, result > 0xFF);
    }

    template<OperandsType SRC_TYPE, typename Operand>
    static constexpr auto INC(gb::SM83CPU* cpu, Operand& src) -> void
    {
        u16 result = 0;

        if constexpr ((SRC_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>))
        {
            src += 1;
            result = src;
        }
        else if constexpr (SRC_TYPE == ADDRESS_PTR)
        {
            result = cpu->read8(src) + 1;
            cpu->write8(src, static_cast<u8>(result));
        }
        else
            assert(false && "Error in INC opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary/immediate variable");

        if constexpr (std::is_same_v<Operand, u8> || SRC_TYPE == ADDRESS_PTR)
        {
            cpu->setFlag(gb::Z, (result & 0x00FF) == 0);
            cpu->setFlag(gb::N, 0);
            cpu->setFlag(gb::H, (((result - 1) & 0x0F) + 1) > 0x0F);
        }
    }

    template<OperandsType SRC_TYPE, typename Operand>
    static constexpr auto DEC_(gb::SM83CPU* cpu, Operand& src) -> void
    {
        u16 result = 0;

        if constexpr ((SRC_TYPE == REGISTER && std::is_unsigned_v<Operand> && not std::is_rvalue_reference_v<Operand>))
        {
            src -= 1;
            result = src;
        }
        else if constexpr (SRC_TYPE == ADDRESS_PTR)
        {
            result = cpu->read8(src) - 1;
            cpu->write8(src, static_cast<u8>(result));
        }
        else
            assert(false && "Error in DEC opcode: Possible errors are wrong OperantType, register passed is not unsigned or it's a temporary/immediate variable");

        if (std::is_same_v<Operand, u8> || SRC_TYPE == ADDRESS_PTR)
        {
            cpu->setFlag(gb::Z, result == 0);
            cpu->setFlag(gb::N, 1);
            cpu->setFlag(gb::H, (((result + 1) & 0x0F) - 1) < 0);
        }
    }

    static auto DAA(gb::SM83CPU* cpu) -> void
    {
        u8 carryFlag = 0;

        // We do high nybble first so that adding 0x06 don't modify upper nybble
        if (cpu->getFlag(gb::C) || ((cpu->regs.A > 0x99) && (cpu->getFlag(gb::N) == 0))) // Also ((cpu->regs.A >> 4) & 0x0F) > 0x09
        {
            cpu->regs.A += (cpu->getFlag(gb::N)) ? -0x60 : 0x60;
            carryFlag = 1;
        }

        if ((((cpu->regs.A & 0x0F) > 0x09) && (cpu->getFlag(gb::N) == 0)) || cpu->getFlag(gb::H))
        {
            cpu->regs.A += (cpu->getFlag(gb::N)) ? -0x06 : 0x06;
        }

        cpu->setFlag(gb::Z, cpu->regs.A == 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, carryFlag);
    }

    static auto CPL(gb::SM83CPU* cpu) -> void
    {
        cpu->regs.A = ~cpu->regs.A;

        cpu->setFlag(gb::N, 1);
        cpu->setFlag(gb::H, 1);
    }

    static auto CCF(gb::SM83CPU* cpu) -> void
    {
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, cpu->getFlag(gb::C) ^ 1);
    }

    static auto SCF(gb::SM83CPU* cpu) -> void
    {
        cpu->setFlag(gb::C, 1);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
    }

    static auto NOP_() -> void
    {
        // Does nothing
    }

    static auto EI(gb::GBConsole* console) -> void
    {
        /*printf("\nIME == %d\n", console->IME ? 1 : 0);
        printf("IF == %02X\n", console->IF.reg);

        printf(" (Executing EI)\n");*/
        //console->IME = true;
        console->getCPU().setInterruptEnablePending();

        //printf("EI executed, pending IME enable\n");

        /*printf("IME == %d\n", console->IME ? 1 : 0);
        printf("IF == %02X\n", console->IF.reg);*/
    }

    static auto DI(gb::GBConsole* console) -> void
    {
        /*printf("\nIME == %d\n", console->IME ? 1 : 0);
        printf("IF == %02X\n", console->IF.reg);

        printf(" (Executing DI)\n");*/

        if (console->getCPU().isInterruptEnablePending())
        {
            console->getCPU().discardInterruptEnablePending();
            //printf("Discarded pending IME enable (delayed EI cancelled)\n");
        }

        console->IME = false;

        //printf("DI executed, IME is %d\n", console->IME);

        /*printf("IME == %d\n", console->IME ? 1 : 0);
        printf("IF == %02X\n", console->IF.reg);*/
    }

    static auto JP(gb::SM83CPU* cpu, bool fromHL = false) -> void
    {
        u16 address = (fromHL) ? cpu->regs.HL : cpu->read16(cpu->regs.PC);
        cpu->regs.PC += 2;
        cpu->regs.PC = address;
    }

    template<JumpConditionFlags condition>
    static auto JP(gb::SM83CPU* cpu) -> u8
    {
        u8 extraCycles = 0;

        u16 address = cpu->read16(cpu->regs.PC);
        cpu->regs.PC += 2;
    
        if ((condition == JP_NZ && cpu->getFlag(Z) == 0)
            || (condition == JP_Z && cpu->getFlag(Z) == 1)
            || (condition == JP_NC && cpu->getFlag(C) == 0)
            || (condition == JP_C && cpu->getFlag(C) == 1))
        {
            extraCycles += 4;
            cpu->regs.PC = address;
        }

        return extraCycles;
    }

    static auto JR(gb::SM83CPU* cpu) -> void
    {
        s8 relativeAddress = static_cast<s8>(cpu->read8(cpu->regs.PC++));
        cpu->regs.PC += relativeAddress;
    }

    template<JumpConditionFlags condition>
    static auto JR(gb::SM83CPU* cpu) -> u8
    {
        u8 extraCycles = 0;

        s8 relativeAddressByte = static_cast<s8>(cpu->read8(cpu->regs.PC++));

        if ((condition == JP_NZ && cpu->getFlag(Z) == 0)
            || (condition == JP_Z && cpu->getFlag(Z) == 1)
            || (condition == JP_NC && cpu->getFlag(C) == 0)
            || (condition == JP_C && cpu->getFlag(C) == 1))
        {
            extraCycles += 4;
            cpu->regs.PC += relativeAddressByte;
        }

        return extraCycles;
    }

    static auto CALL(gb::SM83CPU* cpu) -> void
    {
        u16 newAddress = cpu->read16(cpu->regs.PC);
        cpu->regs.PC += 2;
        PUSH(cpu, cpu->regs.PC);
        cpu->regs.PC = newAddress;
    }

    template<JumpConditionFlags condition>
    static auto CALL(gb::SM83CPU* cpu) -> u8
    {
        u8 extraCycles = 0;

        u16 newAddress = cpu->read16(cpu->regs.PC);
        cpu->regs.PC += 2;

        if ((condition == JP_NZ && cpu->getFlag(Z) == 0)
            || (condition == JP_Z && cpu->getFlag(Z) == 1)
            || (condition == JP_NC && cpu->getFlag(C) == 0)
            || (condition == JP_C && cpu->getFlag(C) == 1))
        {
            extraCycles += 12;
            PUSH(cpu, cpu->regs.PC);
            cpu->regs.PC = newAddress;
        }

        return extraCycles;
    }

    static auto RET(gb::SM83CPU* cpu) -> void
    {
        POP(cpu, cpu->regs.PC);
    }

    template<JumpConditionFlags condition>
    static auto RET(gb::SM83CPU* cpu) -> u8
    {
        u8 extraCycles = 0;

        if ((condition == JP_NZ && cpu->getFlag(Z) == 0)
            || (condition == JP_Z && cpu->getFlag(Z) == 1)
            || (condition == JP_NC && cpu->getFlag(C) == 0)
            || (condition == JP_C && cpu->getFlag(C) == 1))
        {
            extraCycles += 12;
            RET(cpu);
        }

        return extraCycles;
    }


    static auto RETI(gb::SM83CPU* cpu) -> void
    {
        RET(cpu);
        EI(cpu->system);

        //printf("RETI executed\n");
    }

    static auto RST(gb::SM83CPU* cpu, const u8& vectorLowByte) -> void
    {
        u16 vectorAddress = 0x0000 | vectorLowByte;
        PUSH(cpu, cpu->regs.PC);
        cpu->regs.PC = vectorAddress;
    }

    static auto HALT(gb::SM83CPU* cpu) -> void
    {
        //while (!cpu->checkPendingInterrupts());

        //if (cpu->system->IME)
        //{
        //    // TODO
        //}

        cpu->system->enterHaltMode();
        cpu->instructionCycles = 0;
        
        //printf("HALT mode entered\n");
    }

    static auto STOP() -> void
    {

    }

    static auto RLCA(gb::SM83CPU* cpu) -> void
    {
        u8 bit7 = (cpu->regs.A & 0x80) >> 7;
        cpu->regs.A <<= 1;
        cpu->regs.A |= bit7;

        cpu->setFlag(gb::Z, 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit7);
    }

    static auto RRCA(gb::SM83CPU* cpu) -> void
    {
        u8 bit0 = cpu->regs.A & 0x01;
        cpu->regs.A >>= 1;
        cpu->regs.A |= (bit0 << 7);

        cpu->setFlag(gb::Z, 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit0);
    }

    static auto RLA(gb::SM83CPU* cpu) -> void
    {
        u8 bit7 = (cpu->regs.A & 0x80) >> 7;
        cpu->regs.A <<= 1;
        cpu->regs.A |= cpu->getFlag(gb::C);

        cpu->setFlag(gb::Z, 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit7);
    }

    static auto RRA(gb::SM83CPU* cpu) -> void
    {
        u8 bit0 = cpu->regs.A & 0x01;
        cpu->regs.A >>= 1;
        cpu->regs.A |= (cpu->getFlag(gb::C) << 7);

        cpu->setFlag(gb::Z, 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit0);
    }
    
    template<typename Operand>
    static constexpr auto RLC(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        u8 bit7 = (value & 0x80) >> 7;
        value <<= 1;
        value |= bit7;

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit7);
    }

    template<typename Operand>
    static constexpr auto RRC(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        u8 bit0 = value & 0x01;
        value >>= 1;
        value |= (bit0 << 7);

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit0);
    }

    template<typename Operand>
    static constexpr auto RL(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        u8 bit7 = (value & 0x80) >> 7;
        value <<= 1;
        value |= cpu->getFlag(gb::C);

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit7);
    }

    template<typename Operand>
    static constexpr auto RR(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        u8 bit0 = value & 0x01;
        value >>= 1;
        value |= (cpu->getFlag(gb::C) << 7);

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit0);
    }

    template<typename Operand>
    static constexpr auto SLA(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        u8 bit7 = (value & 0x80) >> 7;
        value <<= 1;

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit7);
    }

    template<typename Operand>
    static constexpr auto SRA(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        s8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = static_cast<s8>(cpu->read8(operand));
        else
            value = static_cast<s8>(operand);

        u8 bit0 = value & 0x01;
        value >>= 1;

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, static_cast<u8>(value));
        else
            operand = static_cast<u8>(value);

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit0);
    }

    template<typename Operand>
    static constexpr auto SWAP(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        value = (value << 4) | (value >> 4);

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, 0);
    }

    template<typename Operand>
    static constexpr auto SRL(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        u8 bit0 = value & 0x01;
        value >>= 1;

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;

        cpu->setFlag(gb::Z, value == 0);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 0);
        cpu->setFlag(gb::C, bit0);
    }

    template<u8 Bit, typename Operand>
    static constexpr auto BIT_(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        cpu->setFlag(gb::Z, ((value >> Bit) & 0x01) ^ 1);
        cpu->setFlag(gb::N, 0);
        cpu->setFlag(gb::H, 1);
    }

    template<u8 Bit, typename Operand>
    static constexpr auto RES(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        value &= ~(1u << Bit);

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;
    }

    template<u8 Bit, typename Operand>
    static constexpr auto SET(gb::SM83CPU* cpu, Operand& operand) -> void
    {
        u8 value = 0;

        if constexpr (std::is_same_v<Operand, u16>)
            value = cpu->read8(operand);
        else
            value = operand;

        value |= (1u << Bit);

        if constexpr (std::is_same_v<Operand, u16>)
            cpu->write8(operand, value);
        else
            operand = value;
    }
}
