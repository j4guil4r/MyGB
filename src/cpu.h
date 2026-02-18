#pragma once

#include "types.h"
#include "bus.h"

class CPU {
public:
    explicit CPU(Bus& busReference);
    ~CPU() = default;

    // Core
    void step();
    void reset();

    long long getCycles () {return cycles;};

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
    Word getAF() const {return (A << 8) | F;}
    Word getBC() const {return (B << 8) | C;}
    Word getDE() const {return (D << 8) | E;}
    Word getHL() const {return (H << 8) | L;}

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

    //====== ARQUITECTURA PARA INSTRUCCIONES ======//
    using OpcodeHandler = void (CPU::*)();

    struct Instruction {
        const char* name;
        OpcodeHandler operate;
        int cycles;
    };

    // Lookup Table
    std::vector<Instruction> instructions;

    // 4. Instrucciones específicas (Aquí irás añadiendo las 256...)
    //    Las agrupamos por funcionalidad para orden.

    void OP_UNKNOWN(); // Para opcodes no implementados
    void OP_NOP();     // 0x00

    // Cargas (Load)
    void OP_LD_BC_d16(); // 0x01
    void OP_LD_BC_A();   // 0x02
    void OP_LD_B_d8();   // 0x06
    void OP_LD_a16_SP(); // 0x08

    // ... etc ...

    // Aritmética
    void OP_ADD_A_B();   // 0x80
    void OP_ADD_A_C();   // 0x81
    // ... etc ...

    // Prefijo CB
    void OP_PREFIX_CB(); // 0xCB (Este manejará su propio switch o sub-tabla)
};