#include <cstdio>
#include <format>

#include "cpu.h"

CPU::CPU(Bus& busReference) : bus(busReference) {
    instructions.resize(256);

    for (int i = 0; i < 256; i++) instructions[i] = { "UNKNOWN", &CPU::OP_UNKNOWN, 0 };

    // =============== BLOQUE DE FUNCIONES ==================//

    // --- Fila 0x00 ---
    instructions[0x00] = { "NOP",          &CPU::OP_NOP,        4  };
    instructions[0x01] = { "LD BC, d16",   &CPU::OP_LD_BC_d16,  12 };
    instructions[0x02] = { "LD (BC), A",   &CPU::OP_LD_BC_A,    8  };
    instructions[0x03] = { "INC BC",       &CPU::OP_INC_BC,     8  };
    instructions[0x04] = { "INC B",        &CPU::OP_INC_B,      4  };
    instructions[0x05] = { "DEC B",        &CPU::OP_DEC_B,      4  };
    instructions[0x06] = { "LD B, d8",     &CPU::OP_LD_B_d8,    8  };
    instructions[0x07] = { "RLCA",         &CPU::OP_RLCA,       4  };
    instructions[0x08] = { "LD (a16), SP", &CPU::OP_LD_a16_SP,  20 };
    instructions[0x09] = { "ADD HL, BC",   &CPU::OP_ADD_HL_BC,  8  };
    instructions[0x0A] = { "LD A, (BC)",   &CPU::OP_LD_A_BC,    8  };
    instructions[0x0B] = { "DEC BC",       &CPU::OP_DEC_BC,     8  };
    instructions[0x0C] = { "INC C",        &CPU::OP_INC_C,      4  };
    instructions[0x0D] = { "DEC C",        &CPU::OP_DEC_C,      4  };
    instructions[0x0E] = { "LD C, d8",     &CPU::OP_LD_C_d8,    8  };
    instructions[0x0F] = { "RRCA",         &CPU::OP_RRCA,       4  };

    // --- Fila 0x10 ---
    instructions[0x10] = { "STOP",         &CPU::OP_STOP,       4  };
    instructions[0x11] = { "LD DE, d16",   &CPU::OP_LD_DE_d16,  12 };
    instructions[0x12] = { "LD (DE), A",   &CPU::OP_LD_DE_A,    8  };
    instructions[0x13] = { "INC DE",       &CPU::OP_INC_DE,     8  };
    instructions[0x14] = { "INC D",        &CPU::OP_INC_D,      4  };
    instructions[0x15] = { "DEC D",        &CPU::OP_DEC_D,      4  };
    instructions[0x16] = { "LD D, d8",     &CPU::OP_LD_D_d8,    8  };
    instructions[0x17] = { "RLA",          &CPU::OP_RLA,        4  };
    instructions[0x18] = { "JR r8",        &CPU::OP_JR_r8,      12 };
    instructions[0x19] = { "ADD HL, DE",   &CPU::OP_ADD_HL_DE,  8  };
    instructions[0x1A] = { "LD A, (DE)",   &CPU::OP_LD_A_DE,    8  };
    instructions[0x1B] = { "DEC DE",       &CPU::OP_DEC_DE,     8  };
    instructions[0x1C] = { "INC E",        &CPU::OP_INC_E,      4  };
    instructions[0x1D] = { "DEC E",        &CPU::OP_DEC_E,      4  };
    instructions[0x1E] = { "LD E, d8",     &CPU::OP_LD_E_d8,    8  };
    instructions[0x1F] = { "RRA",          &CPU::OP_RRA,        4  };

    // --- Fila 0x20 ---
    instructions[0x20] = { "JR NZ, r8",    &CPU::OP_JR_NZ_r8,   8  }; // +4 si salta
    instructions[0x21] = { "LD HL, d16",   &CPU::OP_LD_HL_d16,  12 };
    instructions[0x22] = { "LD (HL+), A",  &CPU::OP_LDI_HL_A,   8  };
    instructions[0x23] = { "INC HL",       &CPU::OP_INC_HL,     8  };
    instructions[0x24] = { "INC H",        &CPU::OP_INC_H,      4  };
    instructions[0x25] = { "DEC H",        &CPU::OP_DEC_H,      4  };
    instructions[0x26] = { "LD H, d8",     &CPU::OP_LD_H_d8,    8  };
    instructions[0x27] = { "DAA",          &CPU::OP_DAA,        4  };
    instructions[0x28] = { "JR Z, r8",     &CPU::OP_JR_Z_r8,    8  }; // +4 si salta
    instructions[0x29] = { "ADD HL, HL",   &CPU::OP_ADD_HL_HL,  8  };
    instructions[0x2A] = { "LD A, (HL+)",  &CPU::OP_LDI_A_HL,   8  };
    instructions[0x2B] = { "DEC HL",       &CPU::OP_DEC_HL,     8  };
    instructions[0x2C] = { "INC L",        &CPU::OP_INC_L,      4  };
    instructions[0x2D] = { "DEC L",        &CPU::OP_DEC_L,      4  };
    instructions[0x2E] = { "LD L, d8",     &CPU::OP_LD_L_d8,    8  };
    instructions[0x2F] = { "CPL",          &CPU::OP_CPL,        4  };

    // --- Fila 0x30 ---
    instructions[0x30] = { "JR NC, r8",    &CPU::OP_JR_NC_r8,   8  }; // +4 si salta
    instructions[0x31] = { "LD SP, d16",   &CPU::OP_LD_SP_d16,  12 };
    instructions[0x32] = { "LD (HL-), A",  &CPU::OP_LDD_HL_A,   8  };
    instructions[0x33] = { "INC SP",       &CPU::OP_INC_SP,     8  };
    instructions[0x34] = { "INC (HL)",     &CPU::OP_INC_aHL,    12 };
    instructions[0x35] = { "DEC (HL)",     &CPU::OP_DEC_aHL,    12 };
    instructions[0x36] = { "LD (HL), d8",  &CPU::OP_LD_aHL_d8,  12 };
    instructions[0x37] = { "SCF",          &CPU::OP_SCF,        4  };
    instructions[0x38] = { "JR C, r8",     &CPU::OP_JR_C_r8,    8  }; // +4 si salta
    instructions[0x39] = { "ADD HL, SP",   &CPU::OP_ADD_HL_SP,  8  };
    instructions[0x3A] = { "LD A, (HL-)",  &CPU::OP_LDD_A_HL,   8  };
    instructions[0x3B] = { "DEC SP",       &CPU::OP_DEC_SP,     8  };
    instructions[0x3C] = { "INC A",        &CPU::OP_INC_A,      4  };
    instructions[0x3D] = { "DEC A",        &CPU::OP_DEC_A,      4  };
    instructions[0x3E] = { "LD A, d8",     &CPU::OP_LD_A_d8,    8  };
    instructions[0x3F] = { "CCF",          &CPU::OP_CCF,        4  };

    // --- Fila 0x40 ---
    instructions[0x40] = { "LD B, B",    &CPU::OP_LD_B_B,   4 };
    instructions[0x41] = { "LD B, C",    &CPU::OP_LD_B_C,   4 };
    instructions[0x42] = { "LD B, D",    &CPU::OP_LD_B_D,   4 };
    instructions[0x43] = { "LD B, E",    &CPU::OP_LD_B_E,   4 };
    instructions[0x44] = { "LD B, H",    &CPU::OP_LD_B_H,   4 };
    instructions[0x45] = { "LD B, L",    &CPU::OP_LD_B_L,   4 };
    instructions[0x46] = { "LD B, (HL)", &CPU::OP_LD_B_aHL, 8 };
    instructions[0x47] = { "LD B, A",    &CPU::OP_LD_B_A,   4 };
    instructions[0x48] = { "LD C, B",    &CPU::OP_LD_C_B,   4 };
    instructions[0x49] = { "LD C, C",    &CPU::OP_LD_C_C,   4 };
    instructions[0x4A] = { "LD C, D",    &CPU::OP_LD_C_D,   4 };
    instructions[0x4B] = { "LD C, E",    &CPU::OP_LD_C_E,   4 };
    instructions[0x4C] = { "LD C, H",    &CPU::OP_LD_C_H,   4 };
    instructions[0x4D] = { "LD C, L",    &CPU::OP_LD_C_L,   4 };
    instructions[0x4E] = { "LD C, (HL)", &CPU::OP_LD_C_aHL, 8 };
    instructions[0x4F] = { "LD C, A",    &CPU::OP_LD_C_A,   4 };

    // --- Fila 0x50 ---
    instructions[0x50] = { "LD D, B",    &CPU::OP_LD_D_B,   4 };
    instructions[0x51] = { "LD D, C",    &CPU::OP_LD_D_C,   4 };
    instructions[0x52] = { "LD D, D",    &CPU::OP_LD_D_D,   4 };
    instructions[0x53] = { "LD D, E",    &CPU::OP_LD_D_E,   4 };
    instructions[0x54] = { "LD D, H",    &CPU::OP_LD_D_H,   4 };
    instructions[0x55] = { "LD D, L",    &CPU::OP_LD_D_L,   4 };
    instructions[0x56] = { "LD D, (HL)", &CPU::OP_LD_D_aHL, 8 };
    instructions[0x57] = { "LD D, A",    &CPU::OP_LD_D_A,   4 };
    instructions[0x58] = { "LD E, B",    &CPU::OP_LD_E_B,   4 };
    instructions[0x59] = { "LD E, C",    &CPU::OP_LD_E_C,   4 };
    instructions[0x5A] = { "LD E, D",    &CPU::OP_LD_E_D,   4 };
    instructions[0x5B] = { "LD E, E",    &CPU::OP_LD_E_E,   4 };
    instructions[0x5C] = { "LD E, H",    &CPU::OP_LD_E_H,   4 };
    instructions[0x5D] = { "LD E, L",    &CPU::OP_LD_E_L,   4 };
    instructions[0x5E] = { "LD E, (HL)", &CPU::OP_LD_E_aHL, 8 };
    instructions[0x5F] = { "LD E, A",    &CPU::OP_LD_E_A,   4 };

    // --- Fila 0x60 ---
    instructions[0x60] = { "LD H, B",    &CPU::OP_LD_H_B,   4 };
    instructions[0x61] = { "LD H, C",    &CPU::OP_LD_H_C,   4 };
    instructions[0x62] = { "LD H, D",    &CPU::OP_LD_H_D,   4 };
    instructions[0x63] = { "LD H, E",    &CPU::OP_LD_H_E,   4 };
    instructions[0x64] = { "LD H, H",    &CPU::OP_LD_H_H,   4 };
    instructions[0x65] = { "LD H, L",    &CPU::OP_LD_H_L,   4 };
    instructions[0x66] = { "LD H, (HL)", &CPU::OP_LD_H_aHL, 8 };
    instructions[0x67] = { "LD H, A",    &CPU::OP_LD_H_A,   4 };
    instructions[0x68] = { "LD L, B",    &CPU::OP_LD_L_B,   4 };
    instructions[0x69] = { "LD L, C",    &CPU::OP_LD_L_C,   4 };
    instructions[0x6A] = { "LD L, D",    &CPU::OP_LD_L_D,   4 };
    instructions[0x6B] = { "LD L, E",    &CPU::OP_LD_L_E,   4 };
    instructions[0x6C] = { "LD L, H",    &CPU::OP_LD_L_H,   4 };
    instructions[0x6D] = { "LD L, L",    &CPU::OP_LD_L_L,   4 };
    instructions[0x6E] = { "LD L, (HL)", &CPU::OP_LD_L_aHL, 8 };
    instructions[0x6F] = { "LD L, A",    &CPU::OP_LD_L_A,   4 };

    // --- Fila 0x70 ---
    instructions[0x70] = { "LD (HL), B", &CPU::OP_LD_aHL_B, 8 };
    instructions[0x71] = { "LD (HL), C", &CPU::OP_LD_aHL_C, 8 };
    instructions[0x72] = { "LD (HL), D", &CPU::OP_LD_aHL_D, 8 };
    instructions[0x73] = { "LD (HL), E", &CPU::OP_LD_aHL_E, 8 };
    instructions[0x74] = { "LD (HL), H", &CPU::OP_LD_aHL_H, 8 };
    instructions[0x75] = { "LD (HL), L", &CPU::OP_LD_aHL_L, 8 };
    instructions[0x76] = { "HALT",       &CPU::OP_HALT,     4 };
    instructions[0x77] = { "LD (HL), A", &CPU::OP_LD_aHL_A, 8 };
    instructions[0x78] = { "LD A, B",    &CPU::OP_LD_A_B,   4 };
    instructions[0x79] = { "LD A, C",    &CPU::OP_LD_A_C,   4 };
    instructions[0x7A] = { "LD A, D",    &CPU::OP_LD_A_D,   4 };
    instructions[0x7B] = { "LD A, E",    &CPU::OP_LD_A_E,   4 };
    instructions[0x7C] = { "LD A, H",    &CPU::OP_LD_A_H,   4 };
    instructions[0x7D] = { "LD A, L",    &CPU::OP_LD_A_L,   4 };
    instructions[0x7E] = { "LD A, (HL)", &CPU::OP_LD_A_aHL, 8 };
    instructions[0x7F] = { "LD A, A",    &CPU::OP_LD_A_A,   4 };

    // --- Fila 0x80 ---
    instructions[0x80] = { "ADD A, B",    &CPU::OP_ADD_A_B,   4 };
    instructions[0x81] = { "ADD A, C",    &CPU::OP_ADD_A_C,   4 };
    instructions[0x82] = { "ADD A, D",    &CPU::OP_ADD_A_D,   4 };
    instructions[0x83] = { "ADD A, E",    &CPU::OP_ADD_A_E,   4 };
    instructions[0x84] = { "ADD A, H",    &CPU::OP_ADD_A_H,   4 };
    instructions[0x85] = { "ADD A, L",    &CPU::OP_ADD_A_L,   4 };
    instructions[0x86] = { "ADD A, (HL)", &CPU::OP_ADD_A_aHL, 8 };
    instructions[0x87] = { "ADD A, A",    &CPU::OP_ADD_A_A,   4 };
    instructions[0x88] = { "ADC A, B",    &CPU::OP_ADC_A_B,   4 };
    instructions[0x89] = { "ADC A, C",    &CPU::OP_ADC_A_C,   4 };
    instructions[0x8A] = { "ADC A, D",    &CPU::OP_ADC_A_D,   4 };
    instructions[0x8B] = { "ADC A, E",    &CPU::OP_ADC_A_E,   4 };
    instructions[0x8C] = { "ADC A, H",    &CPU::OP_ADC_A_H,   4 };
    instructions[0x8D] = { "ADC A, L",    &CPU::OP_ADC_A_L,   4 };
    instructions[0x8E] = { "ADC A, (HL)", &CPU::OP_ADC_A_aHL, 8 };
    instructions[0x8F] = { "ADC A, A",    &CPU::OP_ADC_A_A,   4 };

    // --- Fila 0x90 ---
    instructions[0x90] = { "SUB B",       &CPU::OP_SUB_B,     4 };
    instructions[0x91] = { "SUB C",       &CPU::OP_SUB_C,     4 };
    instructions[0x92] = { "SUB D",       &CPU::OP_SUB_D,     4 };
    instructions[0x93] = { "SUB E",       &CPU::OP_SUB_E,     4 };
    instructions[0x94] = { "SUB H",       &CPU::OP_SUB_H,     4 };
    instructions[0x95] = { "SUB L",       &CPU::OP_SUB_L,     4 };
    instructions[0x96] = { "SUB (HL)",    &CPU::OP_SUB_aHL,   8 };
    instructions[0x97] = { "SUB A",       &CPU::OP_SUB_A,     4 };
    instructions[0x98] = { "SBC A, B",    &CPU::OP_SBC_A_B,   4 };
    instructions[0x99] = { "SBC A, C",    &CPU::OP_SBC_A_C,   4 };
    instructions[0x9A] = { "SBC A, D",    &CPU::OP_SBC_A_D,   4 };
    instructions[0x9B] = { "SBC A, E",    &CPU::OP_SBC_A_E,   4 };
    instructions[0x9C] = { "SBC A, H",    &CPU::OP_SBC_A_H,   4 };
    instructions[0x9D] = { "SBC A, L",    &CPU::OP_SBC_A_L,   4 };
    instructions[0x9E] = { "SBC A, (HL)", &CPU::OP_SBC_A_aHL, 8 };
    instructions[0x9F] = { "SBC A, A",    &CPU::OP_SBC_A_A,   4 };

    // --- Fila 0xA0 ---
    instructions[0xA0] = { "AND B",       &CPU::OP_AND_B,     4 };
    instructions[0xA1] = { "AND C",       &CPU::OP_AND_C,     4 };
    instructions[0xA2] = { "AND D",       &CPU::OP_AND_D,     4 };
    instructions[0xA3] = { "AND E",       &CPU::OP_AND_E,     4 };
    instructions[0xA4] = { "AND H",       &CPU::OP_AND_H,     4 };
    instructions[0xA5] = { "AND L",       &CPU::OP_AND_L,     4 };
    instructions[0xA6] = { "AND (HL)",    &CPU::OP_AND_aHL,   8 };
    instructions[0xA7] = { "AND A",       &CPU::OP_AND_A,     4 };
    instructions[0xA8] = { "XOR B",       &CPU::OP_XOR_B,     4 };
    instructions[0xA9] = { "XOR C",       &CPU::OP_XOR_C,     4 };
    instructions[0xAA] = { "XOR D",       &CPU::OP_XOR_D,     4 };
    instructions[0xAB] = { "XOR E",       &CPU::OP_XOR_E,     4 };
    instructions[0xAC] = { "XOR H",       &CPU::OP_XOR_H,     4 };
    instructions[0xAD] = { "XOR L",       &CPU::OP_XOR_L,     4 };
    instructions[0xAE] = { "XOR (HL)",    &CPU::OP_XOR_aHL,   8 };
    instructions[0xAF] = { "XOR A",       &CPU::OP_XOR_A,     4 };

    // --- Fila 0xB0 ---
    instructions[0xB0] = { "OR B",        &CPU::OP_OR_B,      4 };
    instructions[0xB1] = { "OR C",        &CPU::OP_OR_C,      4 };
    instructions[0xB2] = { "OR D",        &CPU::OP_OR_D,      4 };
    instructions[0xB3] = { "OR E",        &CPU::OP_OR_E,      4 };
    instructions[0xB4] = { "OR H",        &CPU::OP_OR_H,      4 };
    instructions[0xB5] = { "OR L",        &CPU::OP_OR_L,      4 };
    instructions[0xB6] = { "OR (HL)",     &CPU::OP_OR_aHL,    8 };
    instructions[0xB7] = { "OR A",        &CPU::OP_OR_A,      4 };
    instructions[0xB8] = { "CP B",        &CPU::OP_CP_B,      4 };
    instructions[0xB9] = { "CP C",        &CPU::OP_CP_C,      4 };
    instructions[0xBA] = { "CP D",        &CPU::OP_CP_D,      4 };
    instructions[0xBB] = { "CP E",        &CPU::OP_CP_E,      4 };
    instructions[0xBC] = { "CP H",        &CPU::OP_CP_H,      4 };
    instructions[0xBD] = { "CP L",        &CPU::OP_CP_L,      4 };
    instructions[0xBE] = { "CP (HL)",     &CPU::OP_CP_aHL,    8 };
    instructions[0xBF] = { "CP A",        &CPU::OP_CP_A,      4 };

}

void CPU::step() {
    if (isStopped) return;
    if (isHalted) {
        cycles += 4;
        return;
    }

    // Fetch
    Byte opcode = fetchByte();

    // Decode & Execute
    Instruction inst = instructions[opcode];
    cycles += inst.cycles;
    (this->*inst.operate)();
}

void CPU::step_OLD() {
    // 0. Halt?
    if (isHalted) {
        // Si estamos en HALT, la CPU no hace nada, pero el reloj sigue corriendo.
        // Consumimos 4 ciclos (1 ciclo de máquina) por "paso" de espera.
        cycles += 4;
        return; // No ejecutamos fetch/decode
    }
    // 1. Fetch
    Byte opcode = fetchByte();

    // 2. Decode & Execute
    switch (opcode) {
        // NOP
        case 0x00:
            cycles += 4;
            break;
        // LD BC, d16 (Opcode 01)
        case 0x01:
            setBC(fetchWord());
            cycles += 12;
            break;
        case 0x05:
            dec(B);
            cycles += 4;
            break;

        // LD DE, d16 (Opcode 11)
        case 0x11:
            setDE(fetchWord());
            cycles += 12;
            break;
            // LD (DE), A (Opcode 12)
        case 0x12:
            bus.write(getDE(), A);
            cycles += 8;
            break;
            // INC DE (Opcode 13) - Incremento de 16 bits
            // Igual que INC HL (23), NO afecta flags.
        case 0x13:
            setDE(getDE() + 1);
            cycles += 8;
            break;
            // INC D (Opcode 14)
        case 0x14:
            inc(D);
            cycles += 4;
            break;

            // INC E (Opcode 1C)
        case 0x1C:
            inc(E);
            cycles += 4;
            break;
            // INC H (Opcode 24)
        case 0x24:
            inc(H);
            cycles += 4;
            break;

            // INC L (Opcode 2C)
        case 0x2C:
            inc(L);
            cycles += 4;
            break;

            // INC (HL) (Opcode 34) - Incremento de 8 bits en memoria
        case 0x34:
        {
            // 1. Leer valor actual de memoria
            Byte val = bus.read(getHL());

            // 2. Incrementar (usamos tu helper que maneja Flags Z, N, H)
            inc(val);

            // 3. Escribir de vuelta
            bus.write(getHL(), val);

            cycles += 12;
        }
            break;

            // LD A, E (Opcode 7B) - Copiar registro E en A
        case 0x7B:
            A = E;
            cycles += 4;
            break;

            // CP d8 (Opcode FE) - Comparar A con un valor inmediato
        case 0xFE:
            cp(fetchByte()); // Leemos el siguiente byte y comparamos
            cycles += 8;
            break;

        // LD C, d8 (Opcode 0E)
        case 0x0E:
            C = fetchByte();
            cycles += 8;
            break;
        // INC B (Opcode 04)
        case 0x04:
            inc(B); // ¡Usa tu helper!
            cycles += 4;
            break;
        // LD B, A (Opcode 47) - Copia A en B
        case 0x47:
            B = A;
            cycles += 4;
            break;

        // LD (HL), A (Opcode 77) - Escribe A en la dirección de memoria HL
        case 0x77:
            bus.write(getHL(), A);
            cycles += 8;
            break;

        // --- HIGH RAM & IO (Sonidos, Pantalla) ---
        // LDH (a8), A (Opcode E0) -> Escribe en 0xFF00 + dato inmediato
        case 0xE0:
        {
            Byte offset = fetchByte();
            Word addr = 0xFF00 | offset; // 0xFF00 + offset
            bus.write(addr, A);
            cycles += 12;
        }
            break;

        // LD (C), A (Opcode E2) -> Escribe en 0xFF00 + Registro C
        // Nota: Aunque se escribe (C), la dirección real es FF00 + C
        case 0xE2:
        {
            Word addr = 0xFF00 | C;
            bus.write(addr, A);
            cycles += 8;
        }
            break;
        case 0x06:
            B = fetchByte();
            cycles += 8;
            break;

        case 0x3E:
            A = fetchByte();
            cycles += 8;
            break;
        case 0x80:
            add(B);
            cycles += 4;
            break;
        case 0xC3: {
            Word targetAddress = fetchWord();
            PC = targetAddress;
            cycles += 16; // ~16/12 en documentacion
        }
            break;
            // LD HL, d16 (Cargar valor de 16 bits en HL)
        case 0x21:
            setHL(fetchWord());
            cycles += 12;
            break;
            // LD A, (HL+) (Opcode 2A) - Lee de (HL) y luego incrementa HL
        case 0x2A:
            A = bus.read(getHL());
            setHL(getHL() + 1);
            cycles += 8;
            break;

            // LD SP, d16 (Inicializar Stack Pointer)
        case 0x31:
            SP = fetchWord();
            cycles += 12;
            break;

            // LD (HL-), A (Escribir A en (HL) y decrementar HL)
        case 0x32:
            bus.write(getHL(), A);
            setHL(getHL() - 1); // Decrementamos después de escribir
            cycles += 8;
            break;

            // XOR A (Opcode AF)
            // Lógica: A = A ^ A (que es 0). Flags: Z=1.
        case 0xAF:
            A = A ^ A; // Obviamente 0, pero seguimos la lógica formal
            setFlag(F_Z, true); // Resultado es cero
            setFlag(F_N, false);
            setFlag(F_H, false);
            setFlag(F_C, false);
            cycles += 4;
            break;
        case 0x0C: inc(C); cycles += 4; break;
        case 0x20:
        {
            int8_t off = (int8_t)fetchByte();
            if(!getFlag(F_Z)) { PC += off; cycles += 12; }
            else { cycles += 8; }
        }
            break;

            // EI (Enable Interrupts) - 0xFB
            // Por ahora no tenemos interrupciones, pero la BIOS lo pide.
        case 0xFB:
            ime = true;
            cycles += 4;
            break;
            // RLA (Opcode 17) - Rotate Left Accumulator
        case 0x17:
        {
            // Usamos el helper rl(A) pero corregimos el flag Z
            rl(A);
            setFlag(F_Z, false); // ¡Regla especial de RLA!
            cycles += 4;
        }
            break;

        // LD C, A (Opcode 4F) - Copiar A en C
        case 0x4F:
            C = A;
            cycles += 4;
            break;

        // POP BC (Opcode C1) - Sacar del Stack a BC
        case 0xC1:
            setBC(popStack());
            cycles += 12;
            break;

        // PUSH BC (Opcode C5) - Guardar BC en el Stack
        case 0xC5:
            pushStack(getBC());
            cycles += 16;
            break;
        case 0xCB:
        {
            Byte cbOp = fetchByte();
            cycles += 4;
            switch(cbOp) {
                // RL C (Opcode 11 dentro de CB) -> Rotate Left C
                case 0x11:
                    rl(C);
                    cycles += 8;
                    break;

                    // BIT 7, H (0x7C) que ya tenías...
                case 0x7C: bit(7, H); cycles += 8; break;
                    // RR C (CB 19)
                case 0x19: rr(C); cycles += 8; break;
                    // RR D (CB 1A)
                case 0x1A: rr(D); cycles += 8; break;

                    // SRL B (CB 38)
                case 0x38: srl(B); cycles += 8; break;

                default:
                    std::cout << std::format("Unimplemented CB: {:02X}\n", cbOp);
            }
        }
            break;
        // LD A, (DE) - Opcode 1A
        // Lee el byte apuntado por el registro DE y lo guarda en A
        case 0x1A:
            A = bus.read(getDE());
            cycles += 8;
            break;

        // SUB L - Opcode 95
        // Resta el valor de L al registro A
        case 0x95:
            sub(L); // Usamos el helper
            cycles += 4;
            break;

        // CALL a16 - Opcode CD
        // Llama a una subrutina
        case 0xCD:
        {
            // 1. Leemos la dirección de destino (donde queremos ir)
            Word targetAddr = fetchWord();

            // 2. Guardamos el PC actual en el Stack para saber volver.
            // IMPORTANTE: El PC ya avanzó (gracias a fetchWord), así que
            // guardamos la dirección de la SIGUIENTE instrucción.
            pushStack(PC);

            // 3. Saltamos
            PC = targetAddr;

            cycles += 24; // Call es costoso
        }
            break;
            // RET (Opcode C9) - Return from Subroutine
        case 0xC9:
            PC = popStack(); // Recuperamos donde estábamos
            cycles += 16;
            break;
            // LD (HL+), A (Opcode 22) - También llamada LDI (Load and Increment)
        case 0x22:
            bus.write(getHL(), A); // Escribir
            setHL(getHL() + 1);    // Incrementar HL
            cycles += 8;
            break;
            // INC HL (Opcode 23) - 16-bit Increment
        case 0x23:
            setHL(getHL() + 1); // Simple suma, SIN tocar flags
            cycles += 8;
            break;
            // DEC C (Opcode 0D)
        case 0x0D:
            dec(C); // Helper con flags correctos
            cycles += 4;
            break;
            // JR r8 (Opcode 18) - Salto Relativo Incondicional
        case 0x18:
        {
            int8_t offset = (int8_t)fetchByte();
            PC += offset;
            cycles += 12;
        }
            break;

            // JR Z, r8 (Opcode 28) - Salto si Z es 1
        case 0x28:
        {
            int8_t offset = (int8_t)fetchByte();
            if (getFlag(F_Z)) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
        }
            break;
            // LDH A, (a8) (Opcode F0) - Leer de 0xFF00 + n
        case 0xF0:
        {
            Byte offset = fetchByte();
            Word addr = 0xFF00 | offset;
            A = bus.read(addr);
            cycles += 12;
        }
            break;
            // LD (a16), A (Opcode EA)
        case 0xEA:
        {
            Word addr = fetchWord();
            bus.write(addr, A);
            cycles += 16;
        }
            break;
            // SBC A, C (Opcode 99)
        case 0x99:
            sbc(C);
            cycles += 4;
            break;
            // STOP (Opcode 10)
        case 0x10:
            fetchByte(); // Consumimos el byte extra (00) que sigue al STOP
            // Aquí deberíamos pausar la CPU, pero por ahora...
            std::cout << "STOP Instruction executed.\n";
            cycles += 4;
            break;
            // DEC A (Opcode 3D)
        case 0x3D:
            dec(A);
            cycles += 4;
            break;
            // LD H, A (Opcode 67) - Copia A en H
        case 0x67:
            H = A;
            cycles += 4;
            break;

            // LD D, A (Opcode 57) - Copia A en D
        case 0x57:
            D = A;
            cycles += 4;
            break;
            // LD E, d8 (Opcode 1E)
        case 0x1E:
            E = fetchByte();
            cycles += 8;
            break;
            // LD (BC), A (Opcode 02)
        case 0x02:
            bus.write(getBC(), A);
            cycles += 8;
            break;
            // LD L, d8 (Opcode 2E)
        case 0x2E:
            L = fetchByte();
            cycles += 8;
            break;
            // RRCA (Opcode 0F) - Rotate Right Circular Accumulator
        case 0x0F:
        {
            // 1. Guardamos el bit 0 (que se va a caer)
            Byte bit0 = A & 0x01;

            // 2. Rotamos: A >> 1 y metemos el bit0 en la posición 7
            A = (A >> 1) | (bit0 << 7);

            // 3. Flags (Truco: Z siempre false en RRCA)
            setFlag(F_Z, false);
            setFlag(F_N, false);
            setFlag(F_H, false);
            setFlag(F_C, bit0); // El bit que salió va al Carry

            cycles += 4;
        }
            break;
            // PUSH HL (Opcode E5)
        case 0xE5:
            pushStack(getHL());
            cycles += 16;
            break;

            // POP HL (Opcode E1)
        case 0xE1:
            setHL(popStack());
            cycles += 12;
            break;

            // PUSH AF (Opcode F5)
        case 0xF5:
            pushStack(getAF());
            cycles += 16;
            break;

            // POP AF (Opcode F1)
        case 0xF1:
        {
            Word af = popStack();
            // LA CORRECCIÓN MÁGICA:
            // Aseguramos que los 4 bits bajos de F (la parte baja de AF) sean 0.
            // 0xFFF0 = 1111 1111 1111 0000
            setAF(af & 0xFFF0);
            cycles += 12;
        }
            break;
            // LD A, B (Opcode 78)
        case 0x78: A = B; cycles += 4; break;

            // LD A, H (Opcode 7C)
        case 0x7C: A = H; cycles += 4; break;

            // LD A, L (Opcode 7D)
        case 0x7D: A = L; cycles += 4; break;
            // AND d8 (Opcode E6) - AND Inmediato
        case 0xE6:
            and_op(fetchByte());
            cycles += 8;
            break;

            // OR C (Opcode B1) - OR con registro C
        case 0xB1:
            or_op(C);
            cycles += 4;
            break;
            // RET C (Opcode D8) - Retorna si Carry es true
        case 0xD8:
            if (getFlag(F_C)) {
                PC = popStack();
                cycles += 20; // Tarda más si salta
            } else {
                cycles += 8;
            }
            break;
            // LD A, (a16) (Opcode FA)
        case 0xFA:
        {
            Word addr = fetchWord();
            A = bus.read(addr);
            cycles += 16;
        }
            break;
            // DI (Opcode F3) - Disable Interrupts
        case 0xF3:
            ime = false;
            cycles += 4;
            break;

            // INC BC (Opcode 03) - 16-bit Increment
        case 0x03:
            setBC(getBC() + 1);
            cycles += 8;
            break;

            // CALL NZ, a16 (Opcode C4) - Llama si Z es 0
        case 0xC4:
        {
            Word target = fetchWord();
            if (!getFlag(F_Z)) {
                pushStack(PC);
                PC = target;
                cycles += 24;
            } else {
                cycles += 12;
            }
        }
            break;

            // CALL Z, a16 (Opcode CC) - Llama si Z es 1
        case 0xCC:
        {
            Word target = fetchWord();
            if (getFlag(F_Z)) {
                pushStack(PC);
                PC = target;
                cycles += 24;
            } else {
                cycles += 12;
            }
        }
            break;

            // CALL NC, a16 (Opcode D4) - Llama si C es 0
        case 0xD4:
        {
            Word target = fetchWord();
            if (!getFlag(F_C)) {
                pushStack(PC);
                PC = target;
                cycles += 24;
            } else {
                cycles += 12;
            }
        }
            break;

            // CALL C, a16 (Opcode DC) - Llama si C es 1
        case 0xDC:
        {
            Word target = fetchWord();
            if (getFlag(F_C)) {
                pushStack(PC);
                PC = target;
                cycles += 24;
            } else {
                cycles += 12;
            }
        }
            break;
            // RET NZ (Opcode C0) - Retorna si Z es 0
        case 0xC0:
            if (!getFlag(F_Z)) {
                PC = popStack();
                cycles += 20;
            } else {
                cycles += 8;
            }
            break;

            // RET Z (Opcode C8) - Retorna si Z es 1
        case 0xC8:
            if (getFlag(F_Z)) {
                PC = popStack();
                cycles += 20;
            } else {
                cycles += 8;
            }
            break;

            // RET NC (Opcode D0) - Retorna si C es 0
        case 0xD0:
            if (!getFlag(F_C)) {
                PC = popStack();
                cycles += 20;
            } else {
                cycles += 8;
            }
            break;
            // DAA (Opcode 27) - Decimal Adjust Accumulator
        case 0x27:
            daa();
            cycles += 4;
            break;
            // CPL (Opcode 2F) - Complement A (Flip bits)
        case 0x2F:
            A = ~A;
            setFlag(F_N, true);
            setFlag(F_H, true);
            cycles += 4;
            break;

            // SCF (Opcode 37) - Set Carry Flag
        case 0x37:
            setFlag(F_N, false);
            setFlag(F_H, false);
            setFlag(F_C, true);
            cycles += 4;
            break;

            // CCF (Opcode 3F) - Complement Carry Flag
        case 0x3F:
            setFlag(F_N, false);
            setFlag(F_H, false);
            setFlag(F_C, !getFlag(F_C)); // Invertimos C
            cycles += 4;
            break;
            // XOR C (Opcode A9)
        case 0xA9:
            xor_op(C);
            cycles += 4;
            break;
            // ADD A, d8 (Opcode C6)
        case 0xC6:
            add(fetchByte());
            cycles += 8;
            break;

            // SUB d8 (Opcode D6)
        case 0xD6:
            sub(fetchByte());
            cycles += 8;
            break;
            // OR A (Opcode B7)
        case 0xB7:
            or_op(A);
            cycles += 4;
            break;
            // JP (HL) (Opcode E9) - PC = HL
        case 0xE9:
            PC = getHL();
            cycles += 4;
            break;
            // RRA (Opcode 1F) - Rotate Right Accumulator
        case 0x1F:
            rr(A);
            setFlag(F_Z, false); // ¡Regla especial de RRA! Z siempre 0
            cycles += 4;
            break;
            // LD (a16), SP (Opcode 08) - Guarda el Stack Pointer en memoria
        case 0x08:
        {
            Word addr = fetchWord();
            // Game Boy es Little Endian: Primero byte bajo, luego alto
            bus.write(addr, SP & 0xFF);
            bus.write(addr + 1, (SP >> 8) & 0xFF);
            cycles += 20;
        }
            break;

            // PUSH DE (Opcode D5)
        case 0xD5:
            pushStack(getDE());
            cycles += 16;
            break;

            // LD B, (HL) (Opcode 46)
        case 0x46:
            B = bus.read(getHL());
            cycles += 8;
            break;

            // LD C, (HL) (Opcode 4E)
        case 0x4E:
            C = bus.read(getHL());
            cycles += 8;
            break;

            // LD D, (HL) (Opcode 56)
        case 0x56:
            D = bus.read(getHL());
            cycles += 8;
            break;

            // LD H, d8 (Opcode 26)
        case 0x26:
            H = fetchByte();
            cycles += 8;
            break;

            // LD A, (BC) (Opcode 0A)
        case 0x0A:
            A = bus.read(getBC());
            cycles += 8;
            break;
            // XOR (HL) (Opcode AE)
        case 0xAE:
            xor_op(bus.read(getHL()));
            cycles += 8;
            break;

            // XOR d8 (Opcode EE)
        case 0xEE:
            xor_op(fetchByte());
            cycles += 8;
            break;

            // DEC L (Opcode 2D)
        case 0x2D:
            dec(L);
            cycles += 4;
            break;

            // ADD A, E (Opcode 83)
        case 0x83:
            add(E);
            cycles += 4;
            break;

            // CP B (Opcode B8)
        case 0xB8:
            cp(B);
            cycles += 4;
            break;
            // JR NC, r8 (Opcode 30) - Salto si No Carry
        case 0x30:
        {
            int8_t offset = (int8_t)fetchByte();
            if (!getFlag(F_C)) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
        }
            break;
            // LD E, A (Opcode 5F)
        case 0x5F:
            E = A;
            cycles += 4;
            break;

            // LD A, C (Opcode 79)
        case 0x79:
            A = C;
            cycles += 4;
            break;

            // LD A, D (Opcode 7A)
        case 0x7A:
            A = D;
            cycles += 4;
            break;
            // DEC H (Opcode 25)
        case 0x25:
            dec(H);
            cycles += 4;
            break;
            // LD (HL), B (Opcode 70)
        case 0x70:
            bus.write(getHL(), B);
            cycles += 8;
            break;

            // LD (HL), C (Opcode 71)
        case 0x71:
            bus.write(getHL(), C);
            cycles += 8;
            break;

            // LD (HL), D (Opcode 72)
        case 0x72:
            bus.write(getHL(), D);
            cycles += 8;
            break;
            // POP DE (Opcode D1)
        case 0xD1:
            setDE(popStack());
            cycles += 12;
            break;
            // ADC A, d8 (Opcode CE)
        case 0xCE:
            adc(fetchByte());
            cycles += 8;
            break;
            // ADD HL, HL (Opcode 29)
        case 0x29:
            addHL(getHL());
            cycles += 8;
            break;

            // DEC (HL) (Opcode 35)
        case 0x35:
        {
            // Read-Modify-Write
            Byte val = bus.read(getHL());
            dec(val); // Helper que maneja flags Z, N, H
            bus.write(getHL(), val);
            cycles += 12;
        }
            break;

            // OR (HL) (Opcode B6)
        case 0xB6:
            or_op(bus.read(getHL()));
            cycles += 8;
            break;

            // LD L, (HL) (Opcode 6E)
        case 0x6E:
            L = bus.read(getHL());
            cycles += 8;
            break;

            // LD L, A (Opcode 6F)
        case 0x6F:
            L = A;
            cycles += 4;
            break;

            // DEC E (Opcode 1D)
        case 0x1D:
            dec(E);
            cycles += 4;
            break;
            // INC A (Opcode 3C)
        case 0x3C:
            inc(A);
            cycles += 4;
            break;
            // CP C (Opcode B9)
        case 0xB9:
            cp(C);
            cycles += 4;
            break;
            // JP NZ, a16 (Opcode C2) - Salta a dirección absoluta si Z es 0
        case 0xC2:
        {
            Word target = fetchWord(); // Leemos la dirección de 16 bits
            if (!getFlag(F_Z)) {
                PC = target;       // Salto absoluto
                cycles += 16;      // Tarda más si salta
            } else {
                cycles += 12;      // Tarda menos si no salta
            }
        }
            break;
            // CP E (Opcode BB)
        case 0xBB:
            cp(E);
            cycles += 4;
            break;
            // LD A, (HL) (Opcode 7E)
        case 0x7E:
            A = bus.read(getHL());
            cycles += 8;
            break;

            // JR C, r8 (Opcode 38) - Salto si Carry es 1
        case 0x38:
        {
            int8_t offset = (int8_t)fetchByte();
            if (getFlag(F_C)) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
        }
            break;

            // DEC BC (Opcode 0B)
        case 0x0B:
            setBC(getBC() - 1);
            cycles += 8;
            break;
            // SUB C (Opcode 91)
        case 0x91:
            sub(C);
            cycles += 4;
            break;

            // ADD A, C (Opcode 81)
        case 0x81:
            add(C);
            cycles += 4;
            break;

            // CP D (Opcode BA)
        case 0xBA:
            cp(D);
            cycles += 4;
            break;
            // LD HL, SP+r8 (Opcode F8)
        case 0xF8:
        {
            // Leemos el desplazamiento con signo (-128 a 127)
            int8_t offset = (int8_t)fetchByte();

            // Calculamos el resultado final
            int result = SP + offset;

            // Flags
            setFlag(F_Z, false); // Z siempre 0 en esta instrucción
            setFlag(F_N, false); // N siempre 0

            // Flag H: Acarreo del bit 3 (como si sumáramos solo bytes)
            // ((SP & 0x0F) + (offset & 0x0F)) > 0x0F
            setFlag(F_H, ((SP & 0x0F) + (offset & 0x0F)) > 0x0F);

            // Flag C: Acarreo del bit 7 (overflow del byte bajo)
            // ((SP & 0xFF) + (offset & 0xFF)) > 0xFF
            setFlag(F_C, ((SP & 0xFF) + (offset & 0xFF)) > 0xFF);

            setHL((Word)result);
            cycles += 12;
        }
            break;
            // JP Z, a16 (Opcode CA) - Salta si Z es 1
        case 0xCA:
        {
            Word target = fetchWord();
            if (getFlag(F_Z)) {
                PC = target;
                cycles += 16;
            } else {
                cycles += 12;
            }
        }
            break;

            // JP NC, a16 (Opcode D2) - Salta si C es 0
        case 0xD2:
        {
            Word target = fetchWord();
            if (!getFlag(F_C)) {
                PC = target;
                cycles += 16;
            } else {
                cycles += 12;
            }
        }
            break;

            // JP C, a16 (Opcode DA) - Salta si C es 1
        case 0xDA:
        {
            Word target = fetchWord();
            if (getFlag(F_C)) {
                PC = target;
                cycles += 16;
            } else {
                cycles += 12;
            }
        }
            break;
            // HALT (Opcode 76) - Pausa la CPU hasta una interrupción
            // HALT (Opcode 76)
        case 0x76:
        {
            // Leemos IE e IF para ver si ya hay una interrupción pendiente
            Byte IE = bus.read(0xFFFF);
            Byte IF = bus.read(0xFF0F);

            // Si hay una interrupción pendiente (y habilitada en IE),
            // HALT no surte efecto (bug del hardware, o simplemente no se duerme).
            if ((IE & IF & 0x1F) != 0) {
                // HALT Bug: En hardware real, esto causa que la siguiente
                // instrucción se lea dos veces. Para emulación simple,
                // basta con NO activar isHalted.
            } else {
                // Si no hay nada pendiente, a dormir.
                isHalted = true;
            }
        }
            cycles += 4;
            break;

        default:
            printf("Unhandled opcode: %02x\n", opcode);
            break;
    }
}

void CPU::reset() {
    PC = 0x0100; // Punto de entrada estándar de la GB
    // TODO: valores por defecto de todos los registros.
}

// FETCH
Byte CPU::fetchByte() {
    Byte data = bus.read(PC);
    PC++;
    return data;
}

Word CPU::fetchWord() {
    Byte data1 = bus.read(PC);
    PC++;
    Byte data2 = bus.read(PC);
    PC++;
    Word data = (data2 << 8) | data1;
    return data;
}

void CPU::add(Byte value) {
    Word result = A + value;

    // Flags
    setFlag(F_Z, (result & 0xFF) == 0);
    setFlag(F_N, false);
    setFlag(F_H, ((A & 0x0F) + (value & 0x0F)) > 0x0F);
    setFlag(F_C, result > 0xFF);

    A = static_cast<Byte>(result & 0xFF);
}

void CPU::inc(Byte& reg) {
    // Antes de sumar, miramos el Half Carry
    // Si los 4 bits bajos son F (15), al sumar 1 habrá desborde al bit 4
    setFlag(F_H, (reg & 0x0F) == 0x0F);

    // Sumamos
    reg++;

    // Flags restantes
    setFlag(F_Z, reg == 0);
    setFlag(F_N, false);
    // INC no toca el Flag C
}

void CPU::dec(Byte& reg) {
    // Half Carry en resta (Borrow): Si los 4 bits bajos son 0, al restar pedirá prestado
    setFlag(F_H, (reg & 0x0F) == 0x00);

    reg--;

    setFlag(F_Z, reg == 0);
    setFlag(F_N, true); // Es una resta
    // DEC tampoco toca C
}

void CPU::bit(int bitIndex, Byte regVal) {
    bool isZero = !((regVal >> bitIndex) & 1);
    setFlag(F_Z, isZero);

    // Flags fijos según manual
    setFlag(F_N, false);
    setFlag(F_H, true); // BIT siempre enciende H
    // C no se toca
}

void CPU::sub(Byte value) {
    // 1. Validar Carry (Borrow) antes de restar
    // Si A es menor que value, necesitaremos un "préstamo", así que Carry = true
    setFlag(F_C, A < value);

    // 2. Validar Half-Carry (Borrow del nibble bajo)
    // ((A & 0xF) - (value & 0xF)) < 0
    setFlag(F_H, (A & 0x0F) < (value & 0x0F));

    // 3. Realizar la resta
    A -= value;

    setFlag(F_Z, A == 0);
    setFlag(F_N, true);
}

void CPU::pushStack(Word value) {
    // El Stack guarda 2 bytes (16 bits).
    // Primero guardamos el byte ALTO, luego el BAJO.
    // Decrementamos SP antes de escribir cada byte.

    // 1. High Byte
    SP--;
    bus.write(SP, (value >> 8) & 0xFF);

    // 2. Low Byte
    SP--;
    bus.write(SP, value & 0xFF);
}

Word CPU::popStack() {
    // 1. Leemos el Byte BAJO primero (porque el Stack es LIFO - Last In First Out)
    Byte lo = bus.read(SP);
    SP++;

    // 2. Leemos el Byte ALTO
    Byte hi = bus.read(SP);
    SP++;

    // 3. Combinamos
    return (hi << 8) | lo;
}

void CPU::rl(Byte& reg) {
    // 1. Guardamos el bit 7 actual (será el nuevo Carry)
    bool isCarry = (reg & 0x80) != 0;

    // 2. Recuperamos el Carry viejo (entrará por la derecha)
    bool oldCarry = getFlag(F_C);

    // 3. Hacemos el shift y metemos el carry viejo en el bit 0
    reg = (reg << 1) | (oldCarry ? 1 : 0);

    // 4. Actualizamos Flags
    setFlag(F_Z, reg == 0); // Z se enciende si el resultado es 0
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, isCarry); // El bit 7 que salió es el nuevo Carry
}

void CPU::cp(Byte value) {
    // La lógica es IDÉNTICA a SUB, pero sin modificar A.

    // 1. Validar Carry (Si A < value, necesitamos préstamo)
    setFlag(F_C, A < value);

    // 2. Validar Half-Carry (Borrow del nibble bajo)
    setFlag(F_H, (A & 0x0F) < (value & 0x0F));

    // 3. Flag Z (Si son iguales, la resta daría 0)
    setFlag(F_Z, A == value);

    // 4. Flag N (Siempre es una resta)
    setFlag(F_N, true);

    // ¡OJO! No hacemos "A -= value". A se queda intacto.
}

void CPU::sbc(Byte value) {
    // Recuperamos el Carry actual (0 o 1)
    Byte carry = getFlag(F_C) ? 1 : 0;

    // Resultado matemático amplio para chequear overflow
    // OJO: Es (A - val - carry)
    int result = A - value - carry;

    // H Flag: Se enciende si hay préstamo en el bit 4
    // La fórmula compleja de Game Boy para esto es:
    setFlag(F_H, ((A & 0x0F) - (value & 0x0F) - carry) < 0);

    // C Flag: Se enciende si hubo préstamo global (resultado negativo)
    setFlag(F_C, result < 0);

    // Z Flag
    setFlag(F_Z, (result & 0xFF) == 0);

    // N Flag: Siempre 1 en resta
    setFlag(F_N, true);

    A = static_cast<Byte>(result & 0xFF);
}

void CPU::and_op(Byte value) {
    A &= value;
    setFlag(F_Z, A == 0);
    setFlag(F_N, false);
    setFlag(F_H, true);  // H = 1
    setFlag(F_C, false);
}

void CPU::or_op(Byte value) {
    A |= value;
    setFlag(F_Z, A == 0);
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, false);
}

void CPU::daa() {
    int correction = 0;

    // Si el flag H está encendido O los 4 bits bajos son mayores a 9
    // Significa que hubo desborde en los decimales bajos
    if (getFlag(F_H) || (!getFlag(F_N) && (A & 0x0F) > 9)) {
        correction |= 0x06;
    }

    // Si el flag C está encendido O el valor total es mayor a 99 (en hex)
    // Significa que hubo desborde en los decimales altos
    if (getFlag(F_C) || (!getFlag(F_N) && A > 0x99)) {
        correction |= 0x60;
        setFlag(F_C, true); // DAA enciende el Carry si corregimos la parte alta
    }

    // Aplicamos la corrección (Sumar o Restar dependiendo del Flag N)
    if (getFlag(F_N)) {
        A -= correction;
    } else {
        A += correction;
    }

    // Flags finales
    setFlag(F_Z, A == 0);
    setFlag(F_H, false); // DAA siempre apaga H
    // El Flag N no se toca
    // El Flag C se actualizó arriba
}

void CPU::xor_op(Byte value) {
    A ^= value;

    // Flags
    setFlag(F_Z, A == 0);
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, false);
}

// Rotate Right
void CPU::rr(Byte& reg) {
    // 1. Guardamos el bit 0 (que caerá al Carry)
    bool isCarry = (reg & 0x01) != 0;

    // 2. Recuperamos el Carry viejo (que entrará por la izquierda, bit 7)
    bool oldCarry = getFlag(F_C);

    // 3. Rotamos: Metemos oldCarry en bit 7 y desplazamos
    reg = (reg >> 1) | (oldCarry ? 0x80 : 0x00);

    // 4. Flags
    setFlag(F_Z, reg == 0);
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, isCarry);
}

// Rotate Left Circular
void CPU::rlc(Byte& reg, bool setZeroFlag) {
    Byte bit7 = reg >> 7;
    reg = (reg << 1) | bit7;

    setFlag(F_C, bit7);
    setFlag(F_N, false);
    setFlag(F_H, false);

    setFlag(F_Z, setZeroFlag && (reg == 0));
}

// Rotate Right Circular
void CPU::rrc(Byte& reg, bool setZeroFlag) {
    Byte bit0 = reg & 0x01;
    reg = (reg >> 1) | (bit0 << 7);

    setFlag(F_C, bit0);
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_Z, setZeroFlag && (reg == 0));
}

// Shift right logical
void CPU::srl(Byte& reg) {
    // 1. Guardamos el bit 0 (que caerá al Carry)
    bool isCarry = (reg & 0x01) != 0;

    // 2. Desplazamos (automáticamente entra 0 por la izquierda al ser unsigned)
    reg >>= 1;

    // 3. Flags
    setFlag(F_Z, reg == 0);
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, isCarry);
}

void CPU::adc(Byte value) {
    // 1. Obtenemos el Carry actual (0 o 1)
    Byte carry = getFlag(F_C) ? 1 : 0;

    // 2. Calculamos resultado completo (int para ver desbordes)
    int result = A + value + carry;

    // 3. Flags
    setFlag(F_Z, (result & 0xFF) == 0);
    setFlag(F_N, false);

    // H Flag: Se enciende si la suma de los nibbles bajos + carry supera 15 (0xF)
    setFlag(F_H, ((A & 0x0F) + (value & 0x0F) + carry) > 0x0F);

    // C Flag: Se enciende si el resultado total no cabe en 8 bits
    setFlag(F_C, result > 0xFF);

    A = static_cast<Byte>(result & 0xFF);
}

void CPU::addHL(Word value) {
    Word hl = getHL();
    int result = hl + value;

    // N Flag: Siempre 0
    setFlag(F_N, false);

    // H Flag: Desborde desde el bit 11
    // ((hl & 0xFFF) + (val & 0xFFF)) > 0xFFF
    setFlag(F_H, (hl & 0x0FFF) + (value & 0x0FFF) > 0x0FFF);

    // C Flag: Desborde desde el bit 15 (mayor a 65535)
    setFlag(F_C, result > 0xFFFF);

    setHL(static_cast<Word>(result & 0xFFFF));
}

void CPU::handleInterrupts() {
    // Leemos IE (Enabled) e IF (Request) desde el Bus
    Byte IE = bus.read(0xFFFF);
    Byte IF = bus.read(0xFF0F);

    if ((IE & IF & 0x10) != 0) {
        isStopped = false;
    }

    if ((IE & IF & 0x1F) != 0) {
        // Esto pasa siempre, tenga IME on u off
        isHalted = false;
    }

    // Si las interrupciones están apagadas, no hacemos nada
    if (!ime) return;

    // Miramos si hay alguna interrupción activa QUE ADEMÁS esté habilitada
    // (Ej: Si VBlank (bit 0) está en 1 en ambos registros)
    if (IE & IF & 0x1F) { // 0x1F son los 5 bits de interrupciones válidos
        ime = false; // Desactivamos interrupciones anidadas automáticamente

        // Consumimos 5 ciclos de reloj (la CPU tarda en reaccionar)

        // Identificamos cual fue (VBlank es la más prioritaria, bit 0)
        Byte interruptMask = IE & IF;

        Word vector = 0x0000;

        // Bit 0: VBlank (INT 40h)
        if (interruptMask & 0x01) {
            bus.write(0xFF0F, IF & ~0x01); // Limpiamos el flag (ack)
            vector = 0x0040;
        }
        // Bit 1: LCD STAT (INT 48h)
        else if (interruptMask & 0x02) {
            bus.write(0xFF0F, IF & ~0x02);
            vector = 0x0048;
        }
        // Bit 2: Timer (INT 50h)
        else if (interruptMask & 0x04) {
            bus.write(0xFF0F, IF & ~0x04);
            vector = 0x0050;
        }
        // Bit 3: Serial (INT 58h)
        else if (interruptMask & 0x08) {
            bus.write(0xFF0F, IF & ~0x08);
            vector = 0x0058;
        }
        // Bit 4: Joypad (INT 60h)
        else if (interruptMask & 0x10) {
            bus.write(0xFF0F, IF & ~0x10);
            vector = 0x0060;
        }

        // Saltar
        pushStack(PC);
        PC = vector;
    }
}

// ========= Nueva tabla ========= //

void CPU::OP_UNKNOWN() {
    PC--;
    step_OLD();
}

// =========================================================
// Opcodes 0x00 - 0x0F
// =========================================================
void CPU::OP_NOP() {}
void CPU::OP_LD_BC_d16() { setBC(fetchWord()); }
void CPU::OP_LD_BC_A()   { bus.write(getBC(), A);}
void CPU::OP_INC_BC()    { setBC(getBC() + 1);}
void CPU::OP_INC_B()     { inc(B); }
void CPU::OP_DEC_B()     { dec(B); }
void CPU::OP_LD_B_d8()   { B = fetchByte(); }
void CPU::OP_RLCA()      { rlc(A, false); }
void CPU::OP_LD_a16_SP() {
    Word addr = fetchWord();
    bus.write(addr, SP);
    bus.write(addr + 1, (SP >> 8));
}
void CPU::OP_ADD_HL_BC() { addHL(getBC()); }
void CPU::OP_LD_A_BC()   { A = bus.read(getBC()); }
void CPU::OP_DEC_BC()    { setBC(getBC() - 1); }
void CPU::OP_INC_C()     { inc(C); }
void CPU::OP_DEC_C()     { dec(C); }
void CPU::OP_LD_C_d8()   { C = fetchByte(); }
void CPU::OP_RRCA()      { rrc(A, false); }

// =========================================================
// Opcodes 0x10 - 0x1F
// =========================================================
void CPU::OP_STOP()      {
    fetchByte();
    isStopped = true;
}
void CPU::OP_LD_DE_d16() { setDE(fetchWord()); }
void CPU::OP_LD_DE_A()   { bus.write(getDE(), A); }
void CPU::OP_INC_DE()    { setDE(getDE() + 1); }
void CPU::OP_INC_D()     { inc(D); }
void CPU::OP_DEC_D()     { dec(D); }
void CPU::OP_LD_D_d8()   { D = fetchByte(); }
void CPU::OP_RLA()       {
    rl(A);
    setFlag(F_Z, false);
}
void CPU::OP_JR_r8()     {
    const auto offset = static_cast<int8_t>(fetchByte());
    PC += offset;
}
void CPU::OP_ADD_HL_DE() { addHL(getDE()); }
void CPU::OP_LD_A_DE()   { A = bus.read(getDE()); }
void CPU::OP_DEC_DE()    { setDE(getDE() - 1); }
void CPU::OP_INC_E()     { inc(E); }
void CPU::OP_DEC_E()     { dec(E); }
void CPU::OP_LD_E_d8()   { E = fetchByte(); }
void CPU::OP_RRA()       {
    rr(A);
    setFlag(F_Z, false);
}

// =========================================================
// Opcodes 0x20 - 0x2F
// =========================================================
void CPU::OP_JR_NZ_r8()  {
    const auto offset = static_cast<int8_t>(fetchByte());

    if (!getFlag(F_Z)) {
        PC += offset;
        cycles += 4;
    }
}
void CPU::OP_LD_HL_d16() { setHL(fetchWord()); }
void CPU::OP_LDI_HL_A() {
    bus.write(getHL(), A);
    setHL(getHL() + 1);
}
void CPU::OP_INC_HL()    { setHL(getHL() + 1); }
void CPU::OP_INC_H()     { inc(H); }
void CPU::OP_DEC_H()     { dec(H); }
void CPU::OP_LD_H_d8()   { H = fetchByte(); }
void CPU::OP_DAA()       { daa(); }
void CPU::OP_JR_Z_r8() {
    const auto offset = static_cast<int8_t>(fetchByte());

    if (getFlag(F_Z)) {
        PC += offset;
        cycles += 4;
    }
}
void CPU::OP_ADD_HL_HL() { addHL(getHL()); }
void CPU::OP_LDI_A_HL()  {
    A = bus.read(getHL());
    setHL(getHL() + 1); // HL++
}
void CPU::OP_DEC_HL()    { setHL(getHL() - 1); }
void CPU::OP_INC_L()     { inc(L);}
void CPU::OP_DEC_L()     { dec(L); }
void CPU::OP_LD_L_d8()   { L = fetchByte(); }
void CPU::OP_CPL() {
    A = ~A; // Complement
    setFlag(F_N, true);
    setFlag(F_H, true);
}

// =========================================================
// Opcodes 0x30 - 0x3F
// =========================================================
void CPU::OP_JR_NC_r8() {
    const auto offset = static_cast<int8_t>(fetchByte());

    if (!getFlag(F_C)) {
        PC += offset;
        cycles += 4;
    }
}
void CPU::OP_LD_SP_d16() { SP = fetchWord(); }
void CPU::OP_LDD_HL_A() {
    bus.write(getHL(), A);
    setHL(getHL() - 1);
}
void CPU::OP_INC_SP()    { SP++; }
void CPU::OP_INC_aHL() {
    Byte val = bus.read(getHL());
    inc(val);
    bus.write(getHL(), val);
}
void CPU::OP_DEC_aHL() {
    Byte val = bus.read(getHL());
    dec(val);
    bus.write(getHL(), val);
}
void CPU::OP_LD_aHL_d8() {
    const Byte val = fetchByte();
    bus.write(getHL(), val);
}
void CPU::OP_SCF() {
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, true);
    // Z no cambia
}
void CPU::OP_JR_C_r8() {
    const auto offset = static_cast<int8_t>(fetchByte());

    if (getFlag(F_C)) {
        PC += offset;
        cycles += 4;
    }
}
void CPU::OP_ADD_HL_SP() { addHL(SP); }
void CPU::OP_LDD_A_HL() {
    A = bus.read(getHL());
    setHL(getHL() - 1);
}
void CPU::OP_DEC_SP()    { SP--; }
void CPU::OP_INC_A()     { inc(A); }
void CPU::OP_DEC_A()     { dec(A); }
void CPU::OP_LD_A_d8()   { A = fetchByte(); }
void CPU::OP_CCF()       {
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, !getFlag(F_C)); // C = ~C
}

// =========================================================
// Opcodes 0x40 - 0x4F
// =========================================================
void CPU::OP_LD_B_B() {B = B;}
void CPU::OP_LD_B_C() {B = C;}
void CPU::OP_LD_B_D() {B = D;}
void CPU::OP_LD_B_E() {B = E;}
void CPU::OP_LD_B_H() {B = H;}
void CPU::OP_LD_B_L() {B = L;}
void CPU::OP_LD_B_aHL(){B = bus.read(getHL());}
void CPU::OP_LD_B_A() {B = A;}
void CPU::OP_LD_C_B() {C = B;}
void CPU::OP_LD_C_C() {C = C;}
void CPU::OP_LD_C_D() {C = D;}
void CPU::OP_LD_C_E() {C = E;}
void CPU::OP_LD_C_H() {C = H;}
void CPU::OP_LD_C_L() {C = L;}
void CPU::OP_LD_C_aHL(){C = bus.read(getHL());}
void CPU::OP_LD_C_A() {C = A;}

// =========================================================
// Opcodes 0x50 - 0x5F
// =========================================================
void CPU::OP_LD_D_B() { D = B; }
void CPU::OP_LD_D_C() { D = C; }
void CPU::OP_LD_D_D() { D = D; }
void CPU::OP_LD_D_E() { D = E; }
void CPU::OP_LD_D_H() { D = H; }
void CPU::OP_LD_D_L() { D = L; }
void CPU::OP_LD_D_aHL() {D = bus.read(getHL());}
void CPU::OP_LD_D_A() {D = A;}
void CPU::OP_LD_E_B() {E = B;}
void CPU::OP_LD_E_C() {E = C;}
void CPU::OP_LD_E_D() {E = D;}
void CPU::OP_LD_E_E() {E = E;}
void CPU::OP_LD_E_H() {E = H;}
void CPU::OP_LD_E_L() {E = L;}
void CPU::OP_LD_E_aHL() { E = bus.read(getHL());}
void CPU::OP_LD_E_A() {E = A;}

// =========================================================
// Opcodes 0x60 - 0x6F
// =========================================================
void CPU::OP_LD_H_B()   { H = B;}
void CPU::OP_LD_H_C()   { H = C;}
void CPU::OP_LD_H_D()   { H = D;}
void CPU::OP_LD_H_E()   { H = E;}
void CPU::OP_LD_H_H()   { H = H;}
void CPU::OP_LD_H_L()   { H = L;}
void CPU::OP_LD_H_aHL() { H = bus.read(getHL()); }
void CPU::OP_LD_H_A()   { H = A; }
void CPU::OP_LD_L_B()   { L = B; }
void CPU::OP_LD_L_C()   { L = C; }
void CPU::OP_LD_L_D()   { L = D; }
void CPU::OP_LD_L_E()   { L = E; }
void CPU::OP_LD_L_H()   { L = H; }
void CPU::OP_LD_L_L()   { L = L; }
void CPU::OP_LD_L_aHL() { L = bus.read(getHL()); }
void CPU::OP_LD_L_A()   { L = A; }

// =========================================================
// Opcodes 0x70 - 0x7F
// =========================================================
void CPU::OP_LD_aHL_B() { bus.write(getHL(), B); }
void CPU::OP_LD_aHL_C() { bus.write(getHL(), C);}
void CPU::OP_LD_aHL_D() { bus.write(getHL(), D);}
void CPU::OP_LD_aHL_E() { bus.write(getHL(), E);}
void CPU::OP_LD_aHL_H() { bus.write(getHL(), H);}
void CPU::OP_LD_aHL_L() { bus.write(getHL(), L);}
void CPU::OP_HALT()     {            
    // Leemos IE e IF para ver si ya hay una interrupción pendiente
    Byte IE = bus.read(0xFFFF);
    Byte IF = bus.read(0xFF0F);

    // Si hay una interrupción pendiente (y habilitada en IE),
    // HALT no surte efecto (bug del hardware, o simplemente no se duerme).
    if ((IE & IF & 0x1F) != 0) {
        // HALT Bug: En hardware real, esto causa que la siguiente
        // instrucción se lea dos veces. Para emulación simple,
        // basta con NO activar isHalted.
    } else {
        // Si no hay nada pendiente, a dormir.
        isHalted = true;
    }
}
void CPU::OP_LD_aHL_A() { bus.write(getHL(), A); }
void CPU::OP_LD_A_B()   { A = B; }
void CPU::OP_LD_A_C()   { A = C; }
void CPU::OP_LD_A_D()   { A = D; }
void CPU::OP_LD_A_E()   { A = E; }
void CPU::OP_LD_A_H()   { A = H; }
void CPU::OP_LD_A_L()   { A = L; }
void CPU::OP_LD_A_aHL() { A = bus.read(getHL());}
void CPU::OP_LD_A_A()   { A = A; }

// =========================================================
// Opcodes 0x80 - 0x8F
// =========================================================
void CPU::OP_ADD_A_B()   {add(B);}
void CPU::OP_ADD_A_C()   {add(C);}
void CPU::OP_ADD_A_D()   {add(D);}
void CPU::OP_ADD_A_E()   {add(E);}
void CPU::OP_ADD_A_H()   {add(H);}
void CPU::OP_ADD_A_L()   {add(L);}
void CPU::OP_ADD_A_aHL() { 
    const Byte val = bus.read(getHL());
    add(val);
}
void CPU::OP_ADD_A_A()   { add(A); }
void CPU::OP_ADC_A_B()   { adc(B); }
void CPU::OP_ADC_A_C()   { adc(C); }
void CPU::OP_ADC_A_D()   { adc(D); }
void CPU::OP_ADC_A_E()   { adc(E); }
void CPU::OP_ADC_A_H()   { adc(H); }
void CPU::OP_ADC_A_L()   { adc(L); }
void CPU::OP_ADC_A_aHL() {
    const Byte val = bus.read(getHL());
    adc(val);
}
void CPU::OP_ADC_A_A()   { adc(A);}

// =========================================================
// Opcodes 0x90 - 0x9F
// =========================================================
void CPU::OP_SUB_B()     { sub(B); }
void CPU::OP_SUB_C()     { sub(C); }
void CPU::OP_SUB_D()     { sub(D); }
void CPU::OP_SUB_E()     { sub(E); }
void CPU::OP_SUB_H()     { sub(H); }
void CPU::OP_SUB_L()     { sub(L); }
void CPU::OP_SUB_aHL() {
    const Byte val = bus.read(getHL());
    sub(val);
}
void CPU::OP_SUB_A()     { sub(A); }
void CPU::OP_SBC_A_B()   { sbc(B); }
void CPU::OP_SBC_A_C()   { sbc(C); }
void CPU::OP_SBC_A_D()   { sbc(D); }
void CPU::OP_SBC_A_E()   { sbc(E); }
void CPU::OP_SBC_A_H()   { sbc(H); }
void CPU::OP_SBC_A_L()   { sbc(L); }
void CPU::OP_SBC_A_aHL() {
    const Byte val = bus.read(getHL());
    sbc(val);
}
void CPU::OP_SBC_A_A()   { sbc(A); }

// =========================================================
// Opcodes 0xA0 - 0xAF
// =========================================================
void CPU::OP_AND_B()   { and_op(B);}
void CPU::OP_AND_C()   { and_op(C); }
void CPU::OP_AND_D()   { and_op(D); }
void CPU::OP_AND_E()   { and_op(E); }
void CPU::OP_AND_H()   { and_op(H); }
void CPU::OP_AND_L()   { and_op(L); }
void CPU::OP_AND_aHL() { and_op(bus.read(getHL()));}
void CPU::OP_AND_A()   { and_op(A);}
void CPU::OP_XOR_B()   { xor_op(B);}
void CPU::OP_XOR_C()   { xor_op(C); }
void CPU::OP_XOR_D()   { xor_op(D); }
void CPU::OP_XOR_E()   { xor_op(E); }
void CPU::OP_XOR_H()   { xor_op(H); }
void CPU::OP_XOR_L()   { xor_op(L); }
void CPU::OP_XOR_aHL() { xor_op(bus.read(getHL())); }
void CPU::OP_XOR_A()   { xor_op(A); }

// =========================================================
// Opcodes 0xB0 - 0xBF
// =========================================================
void CPU::OP_OR_B()    { or_op(B);}
void CPU::OP_OR_C()    { or_op(C); }
void CPU::OP_OR_D()    { or_op(D); }
void CPU::OP_OR_E()    { or_op(E); }
void CPU::OP_OR_H()    { or_op(H); }
void CPU::OP_OR_L()    { or_op(L); }
void CPU::OP_OR_aHL()  { or_op(bus.read(getHL())); }
void CPU::OP_OR_A()    { or_op(A); }
void CPU::OP_CP_B()    { cp(B);}
void CPU::OP_CP_C()    { cp(C); }
void CPU::OP_CP_D()    { cp(D); }
void CPU::OP_CP_E()    { cp(E); }
void CPU::OP_CP_H()    { cp(H); }
void CPU::OP_CP_L()    { cp(L); }
void CPU::OP_CP_aHL()  { cp(bus.read(getHL())); }
void CPU::OP_CP_A()    { cp(A); }