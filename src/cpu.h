#pragma once

#include "types.h"
#include "bus.h"

class CPU {
public:
    explicit CPU(Bus& busReference);
    ~CPU() = default;

    // Core
    void step();
    void step_OLD();
    void reset();

    [[nodiscard]] long long getCycles () const {return cycles;};

    // Registros:
    // TODO: idealmente deben de ser privados.
    Byte A = 0x00; // Acumulador
    Byte F = 0x00; // Flags
    Byte B = 0x00; Byte C = 0x00;
    Byte D = 0x00; Byte E = 0x00;
    Byte H = 0x00; Byte L = 0x00;

    Word SP = 0x0000;
    Word PC = 0x0000;

    // Registros adicionales (union de los registros)
    [[nodiscard]] Word getAF() const {return (A << 8) | F;}
    [[nodiscard]] Word getBC() const {return (B << 8) | C;}
    [[nodiscard]] Word getDE() const {return (D << 8) | E;}
    [[nodiscard]] Word getHL() const {return (H << 8) | L;}

    // Separan los 16 bits en dos de 8.
    void setAF(Word v) { A = (v >> 8); F = v & 0x00F0; } // F tiene 4 bits bajos siempre en 0 por definicion
    void setBC(Word v) { B = (v >> 8); C = v & 0x00FF; }
    void setDE(Word v) { D = (v >> 8); E = v & 0x00FF; }
    void setHL(Word v) { H = (v >> 8); L = v & 0x00FF; }

    // Manejo de Flags
    enum Flag {
        F_Z = (1 << 7), // Zero Flag
        F_N = (1 << 6), // Subtract Flag
        F_H = (1 << 5), // Half-Carry Flag
        F_C = (1 << 4)  // Carry Flag
    };

    // Devuelve true si el flag está encendido
    bool getFlag(const Flag f) const {
        return (F & f) != 0;
    }

    // Enciende o apaga un flag específico
    void setFlag(const Flag f, bool v) {
        if (v) F |= f;
        else   F &= ~f;
    }
    void handleInterrupts();
private:
    Bus& bus;

    // Ciclos que demora
    // Nota: El procesador es Multi-cycle.
    long long cycles = 0;
    bool ime = false; // Interrupt Master Enable
    bool isHalted = false;
    bool isStopped = false;

    // ============= HELPERS ===============//
    Byte fetchByte(); // Lee byte (8-bits) en PC y hace PC++
    Word fetchWord(); // Lee 2 bytes (16-bits) en PC y hace PC+=2

    void add(Byte value);
    void adc(Byte value);
    void sub(Byte value);
    void sbc(Byte value);
    void and_op(Byte value);
    void or_op(Byte value);
    void xor_op(Byte value);
    void cp(Byte value);
    void inc(Byte& reg);
    void dec(Byte& reg);

    // 16-bits:
    void addHL(Word value);
    void pushStack(Word value);
    Word popStack();

    // Bits / Rotaciones
    void bit(int bitIndex, Byte regVal);
    void rl(Byte& reg);
    void rr(Byte &reg);
    void srl(Byte &reg);
    void daa();
    void rlc(Byte &reg, bool setZeroFlag);
    void rrc(Byte &reg, bool setZeroFlag);

    //====== ARQUITECTURA PARA INSTRUCCIONES ======//
    using OpcodeHandler = void (CPU::*)();

    struct Instruction {
        const char* name;
        OpcodeHandler operate;
        int cycles;
    };

    // Lookup Table
    std::vector<Instruction> instructions;

    void OP_UNKNOWN();

    // --- Opcodes 0x00 a 0x0F ---
    void OP_NOP();          // 0x00
    void OP_LD_BC_d16();    // 0x01
    void OP_LD_BC_A();      // 0x02
    void OP_INC_BC();       // 0x03
    void OP_INC_B();        // 0x04
    void OP_DEC_B();        // 0x05
    void OP_LD_B_d8();      // 0x06
    void OP_RLCA();         // 0x07
    void OP_LD_a16_SP();    // 0x08
    void OP_ADD_HL_BC();    // 0x09
    void OP_LD_A_BC();      // 0x0A
    void OP_DEC_BC();       // 0x0B
    void OP_INC_C();        // 0x0C
    void OP_DEC_C();        // 0x0D
    void OP_LD_C_d8();      // 0x0E
    void OP_RRCA();         // 0x0F

    // --- Opcodes 0x10 a 0x1F ---
    void OP_STOP();         // 0x10
    void OP_LD_DE_d16();    // 0x11
    void OP_LD_DE_A();      // 0x12
    void OP_INC_DE();       // 0x13
    void OP_INC_D();        // 0x14
    void OP_DEC_D();        // 0x15
    void OP_LD_D_d8();      // 0x16
    void OP_RLA();          // 0x17
    void OP_JR_r8();        // 0x18
    void OP_ADD_HL_DE();    // 0x19
    void OP_LD_A_DE();      // 0x1A
    void OP_DEC_DE();       // 0x1B
    void OP_INC_E();        // 0x1C
    void OP_DEC_E();        // 0x1D
    void OP_LD_E_d8();      // 0x1E
    void OP_RRA();          // 0x1F

    // --- Opcodes 0x20 a 0x2F ---
    void OP_JR_NZ_r8();     // 0x20
    void OP_LD_HL_d16();    // 0x21
    void OP_LDI_HL_A();     // 0x22 (LD (HL+), A)
    void OP_INC_HL();       // 0x23
    void OP_INC_H();        // 0x24
    void OP_DEC_H();        // 0x25
    void OP_LD_H_d8();      // 0x26
    void OP_DAA();          // 0x27
    void OP_JR_Z_r8();      // 0x28
    void OP_ADD_HL_HL();    // 0x29
    void OP_LDI_A_HL();     // 0x2A (LD A, (HL+))
    void OP_DEC_HL();       // 0x2B
    void OP_INC_L();        // 0x2C
    void OP_DEC_L();        // 0x2D
    void OP_LD_L_d8();      // 0x2E
    void OP_CPL();          // 0x2F

    // --- Opcodes 0x30 a 0x3F ---
    void OP_JR_NC_r8();     // 0x30
    void OP_LD_SP_d16();    // 0x31
    void OP_LDD_HL_A();     // 0x32 (LD (HL-), A)
    void OP_INC_SP();       // 0x33
    void OP_INC_aHL();      // 0x34 (INC (HL))
    void OP_DEC_aHL();      // 0x35 (DEC (HL))
    void OP_LD_aHL_d8();    // 0x36 (LD (HL), d8)
    void OP_SCF();          // 0x37
    void OP_JR_C_r8();      // 0x38
    void OP_ADD_HL_SP();    // 0x39
    void OP_LDD_A_HL();     // 0x3A (LD A, (HL-))
    void OP_DEC_SP();       // 0x3B
    void OP_INC_A();        // 0x3C
    void OP_DEC_A();        // 0x3D
    void OP_LD_A_d8();      // 0x3E
    void OP_CCF();          // 0x3F


    void OP_PREFIX_CB(); // 0xCB (Este manejará su propio switch o sub-tabla)
};