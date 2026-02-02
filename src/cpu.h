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
private:
    Bus& bus;

    // Ciclos que demora
    // Nota: El procesador es Multi-cycle.
    uint8_t cycles = 0;

    Byte fetchByte(); // Lee byte (8-bits) en PC y hace PC++
    Word fetchWord(); // Lee 2 bytes (16-bits) en PC y hace PC+=2
};