#include <cstdio>
#include <format>
#include <stdexcept>

#include "cpu.h"

CPU::CPU(Bus& busReference) : bus(busReference) {
    instructions.resize(256);

    for (int i = 0; i < 256; i++) instructions[i] = { "UNKNOWN", &CPU::OP_UNKNOWN, 0 };

    for (int i = 0; i < 256; i++) cb_instructions[i] = {"UNKNOWN_CB", &CPU::OP_UNKNOWN_CB, 0};

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

    // --- Fila 0xC0 ---
    instructions[0xC0] = { "RET NZ",       &CPU::OP_RET_NZ,      8  }; // +12 si retorna
    instructions[0xC1] = { "POP BC",       &CPU::OP_POP_BC,      12 };
    instructions[0xC2] = { "JP NZ, a16",   &CPU::OP_JP_NZ_a16,   12 }; // +4 si salta
    instructions[0xC3] = { "JP a16",       &CPU::OP_JP_a16,      16 };
    instructions[0xC4] = { "CALL NZ, a16", &CPU::OP_CALL_NZ_a16, 12 }; // +12 si llama
    instructions[0xC5] = { "PUSH BC",      &CPU::OP_PUSH_BC,     16 };
    instructions[0xC6] = { "ADD A, d8",    &CPU::OP_ADD_A_d8,    8  };
    instructions[0xC7] = { "RST 00H",      &CPU::OP_RST_00H,     16 };
    instructions[0xC8] = { "RET Z",        &CPU::OP_RET_Z,       8  }; // +12 si retorna
    instructions[0xC9] = { "RET",          &CPU::OP_RET,         16 };
    instructions[0xCA] = { "JP Z, a16",    &CPU::OP_JP_Z_a16,    12 }; // +4 si salta
    instructions[0xCB] = { "PREFIX CB",    &CPU::OP_PREFIX_CB,   4  }; // El prefijo mágico
    instructions[0xCC] = { "CALL Z, a16",  &CPU::OP_CALL_Z_a16,  12 }; // +12 si llama
    instructions[0xCD] = { "CALL a16",     &CPU::OP_CALL_a16,    24 };
    instructions[0xCE] = { "ADC A, d8",    &CPU::OP_ADC_A_d8,    8  };
    instructions[0xCF] = { "RST 08H",      &CPU::OP_RST_08H,     16 };

    // --- Fila 0xD0 ---
    instructions[0xD0] = { "RET NC",       &CPU::OP_RET_NC,      8  }; // +12 si retorna
    instructions[0xD1] = { "POP DE",       &CPU::OP_POP_DE,      12 };
    instructions[0xD2] = { "JP NC, a16",   &CPU::OP_JP_NC_a16,   12 }; // +4 si salta
    // 0xD3 no se asigna, queda como UNKNOWN (Ilegal)
    instructions[0xD4] = { "CALL NC, a16", &CPU::OP_CALL_NC_a16, 12 }; // +12 si llama
    instructions[0xD5] = { "PUSH DE",      &CPU::OP_PUSH_DE,     16 };
    instructions[0xD6] = { "SUB d8",       &CPU::OP_SUB_d8,      8  };
    instructions[0xD7] = { "RST 10H",      &CPU::OP_RST_10H,     16 };
    instructions[0xD8] = { "RET C",        &CPU::OP_RET_C,       8  }; // +12 si retorna
    instructions[0xD9] = { "RETI",         &CPU::OP_RETI,        16 };
    instructions[0xDA] = { "JP C, a16",    &CPU::OP_JP_C_a16,    12 }; // +4 si salta
    // 0xDB no se asigna, queda como UNKNOWN (Ilegal)
    instructions[0xDC] = { "CALL C, a16",  &CPU::OP_CALL_C_a16,  12 }; // +12 si llama
    // 0xDD no se asigna, queda como UNKNOWN (Ilegal)
    instructions[0xDE] = { "SBC A, d8",    &CPU::OP_SBC_A_d8,    8  };
    instructions[0xDF] = { "RST 18H",      &CPU::OP_RST_18H,     16 };

    // --- Fila 0xE0 ---
    instructions[0xE0] = { "LDH (a8), A",  &CPU::OP_LDH_a8_A,    12 };
    instructions[0xE1] = { "POP HL",       &CPU::OP_POP_HL,      12 };
    instructions[0xE2] = { "LD (C), A",    &CPU::OP_LD_C_A_BUS,      8  };
    // 0xE3 y 0xE4 quedan como UNKNOWN  
    instructions[0xE5] = { "PUSH HL",      &CPU::OP_PUSH_HL,     16 };
    instructions[0xE6] = { "AND d8",       &CPU::OP_AND_d8,      8  };
    instructions[0xE7] = { "RST 20H",      &CPU::OP_RST_20H,     16 };
    instructions[0xE8] = { "ADD SP, r8",   &CPU::OP_ADD_SP_r8,   16 };
    instructions[0xE9] = { "JP (HL)",      &CPU::OP_JP_HL,       4  };
    instructions[0xEA] = { "LD (a16), A",  &CPU::OP_LD_a16_A,    16 };
    // 0xEB, 0xEC, 0xED quedan como UNKNOWN
    instructions[0xEF] = { "RST 28H",      &CPU::OP_RST_28H,     16 };
    instructions[0xEE] = { "XOR d8",       &CPU::OP_XOR_d8,      8  };

    // --- Fila 0xF0 ---
    instructions[0xF0] = { "LDH A, (a8)",  &CPU::OP_LDH_A_a8,    12 };
    instructions[0xF1] = { "POP AF",       &CPU::OP_POP_AF,      12 };
    instructions[0xF2] = { "LD A, (C)",    &CPU::OP_LD_A_C_BUS,  8  };
    instructions[0xF3] = { "DI",           &CPU::OP_DI,          4  };
    // 0xF4 queda como UNKNOWN
    instructions[0xF5] = { "PUSH AF",      &CPU::OP_PUSH_AF,     16 };
    instructions[0xF6] = { "OR d8",        &CPU::OP_OR_d8,       8  };
    instructions[0xF7] = { "RST 30H",      &CPU::OP_RST_30H,     16 };
    instructions[0xF8] = { "LD HL, SP+r8", &CPU::OP_LD_HL_SP_r8, 12 };
    instructions[0xF9] = { "LD SP, HL",    &CPU::OP_LD_SP_HL,    8  };
    instructions[0xFA] = { "LD A, (a16)",  &CPU::OP_LD_A_a16,    16 };
    instructions[0xFB] = { "EI",           &CPU::OP_EI,          4  };
    // 0xFC, 0xFD quedan como UNKNOWN
    instructions[0xFE] = { "CP d8",        &CPU::OP_CP_d8,       8  };
    instructions[0xFF] = { "RST 38H",      &CPU::OP_RST_38H,     16 };

    // =============== BLOQUE DE FUNCIONES CB ==================//

    cb_instructions[0x00] = { "RLC B",    &CPU::OP_CB_RLC_B,   8  };
    cb_instructions[0x01] = { "RLC C",    &CPU::OP_CB_RLC_C,   8  };
    cb_instructions[0x02] = { "RLC D",    &CPU::OP_CB_RLC_D,   8  };
    cb_instructions[0x03] = { "RLC E",    &CPU::OP_CB_RLC_E,   8  };
    cb_instructions[0x04] = { "RLC H",    &CPU::OP_CB_RLC_H,   8  };
    cb_instructions[0x05] = { "RLC L",    &CPU::OP_CB_RLC_L,   8  };
    cb_instructions[0x06] = { "RLC (HL)", &CPU::OP_CB_RLC_aHL, 16 };
    cb_instructions[0x07] = { "RLC A",    &CPU::OP_CB_RLC_A,   8  };
    
    cb_instructions[0x08] = { "RRC B",    &CPU::OP_CB_RRC_B,   8  };
    cb_instructions[0x09] = { "RRC C",    &CPU::OP_CB_RRC_C,   8  };
    cb_instructions[0x0A] = { "RRC D",    &CPU::OP_CB_RRC_D,   8  };
    cb_instructions[0x0B] = { "RRC E",    &CPU::OP_CB_RRC_E,   8  };
    cb_instructions[0x0C] = { "RRC H",    &CPU::OP_CB_RRC_H,   8  };
    cb_instructions[0x0D] = { "RRC L",    &CPU::OP_CB_RRC_L,   8  };
    cb_instructions[0x0E] = { "RRC (HL)", &CPU::OP_CB_RRC_aHL, 16 };
    cb_instructions[0x0F] = { "RRC A",    &CPU::OP_CB_RRC_A,   8  };

    // --- Fila CB 0x10 ---
    cb_instructions[0x10] = { "RL B",     &CPU::OP_CB_RL_B,    8  };
    cb_instructions[0x11] = { "RL C",     &CPU::OP_CB_RL_C,    8  };
    cb_instructions[0x12] = { "RL D",     &CPU::OP_CB_RL_D,    8  };
    cb_instructions[0x13] = { "RL E",     &CPU::OP_CB_RL_E,    8  };
    cb_instructions[0x14] = { "RL H",     &CPU::OP_CB_RL_H,    8  };
    cb_instructions[0x15] = { "RL L",     &CPU::OP_CB_RL_L,    8  };
    cb_instructions[0x16] = { "RL (HL)",  &CPU::OP_CB_RL_aHL,  16 };
    cb_instructions[0x17] = { "RL A",     &CPU::OP_CB_RL_A,    8  };

    cb_instructions[0x18] = { "RR B",     &CPU::OP_CB_RR_B,    8  };
    cb_instructions[0x19] = { "RR C",     &CPU::OP_CB_RR_C,    8  };
    cb_instructions[0x1A] = { "RR D",     &CPU::OP_CB_RR_D,    8  };
    cb_instructions[0x1B] = { "RR E",     &CPU::OP_CB_RR_E,    8  };
    cb_instructions[0x1C] = { "RR H",     &CPU::OP_CB_RR_H,    8  };
    cb_instructions[0x1D] = { "RR L",     &CPU::OP_CB_RR_L,    8  };
    cb_instructions[0x1E] = { "RR (HL)",  &CPU::OP_CB_RR_aHL,  16 };
    cb_instructions[0x1F] = { "RR A",     &CPU::OP_CB_RR_A,    8  };

    // --- Fila CB 0x20 ---
    cb_instructions[0x20] = { "SLA B",    &CPU::OP_CB_SLA_B,   8  };
    cb_instructions[0x21] = { "SLA C",    &CPU::OP_CB_SLA_C,   8  };
    cb_instructions[0x22] = { "SLA D",    &CPU::OP_CB_SLA_D,   8  };
    cb_instructions[0x23] = { "SLA E",    &CPU::OP_CB_SLA_E,   8  };
    cb_instructions[0x24] = { "SLA H",    &CPU::OP_CB_SLA_H,   8  };
    cb_instructions[0x25] = { "SLA L",    &CPU::OP_CB_SLA_L,   8  };
    cb_instructions[0x26] = { "SLA (HL)", &CPU::OP_CB_SLA_aHL, 16 };
    cb_instructions[0x27] = { "SLA A",    &CPU::OP_CB_SLA_A,   8  };

    cb_instructions[0x28] = { "SRA B",    &CPU::OP_CB_SRA_B,   8  };
    cb_instructions[0x29] = { "SRA C",    &CPU::OP_CB_SRA_C,   8  };
    cb_instructions[0x2A] = { "SRA D",    &CPU::OP_CB_SRA_D,   8  };
    cb_instructions[0x2B] = { "SRA E",    &CPU::OP_CB_SRA_E,   8  };
    cb_instructions[0x2C] = { "SRA H",    &CPU::OP_CB_SRA_H,   8  };
    cb_instructions[0x2D] = { "SRA L",    &CPU::OP_CB_SRA_L,   8  };
    cb_instructions[0x2E] = { "SRA (HL)", &CPU::OP_CB_SRA_aHL, 16 };
    cb_instructions[0x2F] = { "SRA A",    &CPU::OP_CB_SRA_A,   8  };

    // --- Fila CB 0x30 ---
    cb_instructions[0x30] = { "SWAP B",    &CPU::OP_CB_SWAP_B,   8  };
    cb_instructions[0x31] = { "SWAP C",    &CPU::OP_CB_SWAP_C,   8  };
    cb_instructions[0x32] = { "SWAP D",    &CPU::OP_CB_SWAP_D,   8  };
    cb_instructions[0x33] = { "SWAP E",    &CPU::OP_CB_SWAP_E,   8  };
    cb_instructions[0x34] = { "SWAP H",    &CPU::OP_CB_SWAP_H,   8  };
    cb_instructions[0x35] = { "SWAP L",    &CPU::OP_CB_SWAP_L,   8  };
    cb_instructions[0x36] = { "SWAP (HL)", &CPU::OP_CB_SWAP_aHL, 16 };
    cb_instructions[0x37] = { "SWAP A",    &CPU::OP_CB_SWAP_A,   8  };

    cb_instructions[0x38] = { "SRL B",     &CPU::OP_CB_SRL_B,    8  };
    cb_instructions[0x39] = { "SRL C",     &CPU::OP_CB_SRL_C,    8  };
    cb_instructions[0x3A] = { "SRL D",     &CPU::OP_CB_SRL_D,    8  };
    cb_instructions[0x3B] = { "SRL E",     &CPU::OP_CB_SRL_E,    8  };
    cb_instructions[0x3C] = { "SRL H",     &CPU::OP_CB_SRL_H,    8  };
    cb_instructions[0x3D] = { "SRL L",     &CPU::OP_CB_SRL_L,    8  };
    cb_instructions[0x3E] = { "SRL (HL)",  &CPU::OP_CB_SRL_aHL,  16 };
    cb_instructions[0x3F] = { "SRL A",     &CPU::OP_CB_SRL_A,    8  };

    // --- Fila CB 0x40 (Bit 0) ---
    cb_instructions[0x40] = { "BIT 0, B", &CPU::OP_CB_BIT_0_B, 8 }; cb_instructions[0x41] = { "BIT 0, C", &CPU::OP_CB_BIT_0_C, 8 };
    cb_instructions[0x42] = { "BIT 0, D", &CPU::OP_CB_BIT_0_D, 8 }; cb_instructions[0x43] = { "BIT 0, E", &CPU::OP_CB_BIT_0_E, 8 };
    cb_instructions[0x44] = { "BIT 0, H", &CPU::OP_CB_BIT_0_H, 8 }; cb_instructions[0x45] = { "BIT 0, L", &CPU::OP_CB_BIT_0_L, 8 };
    cb_instructions[0x46] = { "BIT 0, (HL)", &CPU::OP_CB_BIT_0_aHL, 12 }; cb_instructions[0x47] = { "BIT 0, A", &CPU::OP_CB_BIT_0_A, 8 };

    // --- Fila CB 0x48 (Bit 1) ---
    cb_instructions[0x48] = { "BIT 1, B", &CPU::OP_CB_BIT_1_B, 8 }; cb_instructions[0x49] = { "BIT 1, C", &CPU::OP_CB_BIT_1_C, 8 };
    cb_instructions[0x4A] = { "BIT 1, D", &CPU::OP_CB_BIT_1_D, 8 }; cb_instructions[0x4B] = { "BIT 1, E", &CPU::OP_CB_BIT_1_E, 8 };
    cb_instructions[0x4C] = { "BIT 1, H", &CPU::OP_CB_BIT_1_H, 8 }; cb_instructions[0x4D] = { "BIT 1, L", &CPU::OP_CB_BIT_1_L, 8 };
    cb_instructions[0x4E] = { "BIT 1, (HL)", &CPU::OP_CB_BIT_1_aHL, 12 }; cb_instructions[0x4F] = { "BIT 1, A", &CPU::OP_CB_BIT_1_A, 8 };

    // --- Fila CB 0x50 (Bit 2) ---
    cb_instructions[0x50] = { "BIT 2, B", &CPU::OP_CB_BIT_2_B, 8 }; cb_instructions[0x51] = { "BIT 2, C", &CPU::OP_CB_BIT_2_C, 8 };
    cb_instructions[0x52] = { "BIT 2, D", &CPU::OP_CB_BIT_2_D, 8 }; cb_instructions[0x53] = { "BIT 2, E", &CPU::OP_CB_BIT_2_E, 8 };
    cb_instructions[0x54] = { "BIT 2, H", &CPU::OP_CB_BIT_2_H, 8 }; cb_instructions[0x55] = { "BIT 2, L", &CPU::OP_CB_BIT_2_L, 8 };
    cb_instructions[0x56] = { "BIT 2, (HL)", &CPU::OP_CB_BIT_2_aHL, 12 }; cb_instructions[0x57] = { "BIT 2, A", &CPU::OP_CB_BIT_2_A, 8 };

    // --- Fila CB 0x58 (Bit 3) ---
    cb_instructions[0x58] = { "BIT 3, B", &CPU::OP_CB_BIT_3_B, 8 }; cb_instructions[0x59] = { "BIT 3, C", &CPU::OP_CB_BIT_3_C, 8 };
    cb_instructions[0x5A] = { "BIT 3, D", &CPU::OP_CB_BIT_3_D, 8 }; cb_instructions[0x5B] = { "BIT 3, E", &CPU::OP_CB_BIT_3_E, 8 };
    cb_instructions[0x5C] = { "BIT 3, H", &CPU::OP_CB_BIT_3_H, 8 }; cb_instructions[0x5D] = { "BIT 3, L", &CPU::OP_CB_BIT_3_L, 8 };
    cb_instructions[0x5E] = { "BIT 3, (HL)", &CPU::OP_CB_BIT_3_aHL, 12 }; cb_instructions[0x5F] = { "BIT 3, A", &CPU::OP_CB_BIT_3_A, 8 };

    // --- Fila CB 0x60 (Bit 4) ---
    cb_instructions[0x60] = { "BIT 4, B", &CPU::OP_CB_BIT_4_B, 8 }; cb_instructions[0x61] = { "BIT 4, C", &CPU::OP_CB_BIT_4_C, 8 };
    cb_instructions[0x62] = { "BIT 4, D", &CPU::OP_CB_BIT_4_D, 8 }; cb_instructions[0x63] = { "BIT 4, E", &CPU::OP_CB_BIT_4_E, 8 };
    cb_instructions[0x64] = { "BIT 4, H", &CPU::OP_CB_BIT_4_H, 8 }; cb_instructions[0x65] = { "BIT 4, L", &CPU::OP_CB_BIT_4_L, 8 };
    cb_instructions[0x66] = { "BIT 4, (HL)", &CPU::OP_CB_BIT_4_aHL, 12 }; cb_instructions[0x67] = { "BIT 4, A", &CPU::OP_CB_BIT_4_A, 8 };

    // --- Fila CB 0x68 (Bit 5) ---
    cb_instructions[0x68] = { "BIT 5, B", &CPU::OP_CB_BIT_5_B, 8 }; cb_instructions[0x69] = { "BIT 5, C", &CPU::OP_CB_BIT_5_C, 8 };
    cb_instructions[0x6A] = { "BIT 5, D", &CPU::OP_CB_BIT_5_D, 8 }; cb_instructions[0x6B] = { "BIT 5, E", &CPU::OP_CB_BIT_5_E, 8 };
    cb_instructions[0x6C] = { "BIT 5, H", &CPU::OP_CB_BIT_5_H, 8 }; cb_instructions[0x6D] = { "BIT 5, L", &CPU::OP_CB_BIT_5_L, 8 };
    cb_instructions[0x6E] = { "BIT 5, (HL)", &CPU::OP_CB_BIT_5_aHL, 12 }; cb_instructions[0x6F] = { "BIT 5, A", &CPU::OP_CB_BIT_5_A, 8 };

    // --- Fila CB 0x70 (Bit 6) ---
    cb_instructions[0x70] = { "BIT 6, B", &CPU::OP_CB_BIT_6_B, 8 }; cb_instructions[0x71] = { "BIT 6, C", &CPU::OP_CB_BIT_6_C, 8 };
    cb_instructions[0x72] = { "BIT 6, D", &CPU::OP_CB_BIT_6_D, 8 }; cb_instructions[0x73] = { "BIT 6, E", &CPU::OP_CB_BIT_6_E, 8 };
    cb_instructions[0x74] = { "BIT 6, H", &CPU::OP_CB_BIT_6_H, 8 }; cb_instructions[0x75] = { "BIT 6, L", &CPU::OP_CB_BIT_6_L, 8 };
    cb_instructions[0x76] = { "BIT 6, (HL)", &CPU::OP_CB_BIT_6_aHL, 12 }; cb_instructions[0x77] = { "BIT 6, A", &CPU::OP_CB_BIT_6_A, 8 };

    // --- Fila CB 0x78 (Bit 7) ---
    cb_instructions[0x78] = { "BIT 7, B", &CPU::OP_CB_BIT_7_B, 8 }; cb_instructions[0x79] = { "BIT 7, C", &CPU::OP_CB_BIT_7_C, 8 };
    cb_instructions[0x7A] = { "BIT 7, D", &CPU::OP_CB_BIT_7_D, 8 }; cb_instructions[0x7B] = { "BIT 7, E", &CPU::OP_CB_BIT_7_E, 8 };
    cb_instructions[0x7C] = { "BIT 7, H", &CPU::OP_CB_BIT_7_H, 8 }; cb_instructions[0x7D] = { "BIT 7, L", &CPU::OP_CB_BIT_7_L, 8 };
    cb_instructions[0x7E] = { "BIT 7, (HL)", &CPU::OP_CB_BIT_7_aHL, 12 }; cb_instructions[0x7F] = { "BIT 7, A", &CPU::OP_CB_BIT_7_A, 8 };

    // --- Fila CB 0x80 (Bit 0) ---
    cb_instructions[0x80] = { "RES 0, B", &CPU::OP_CB_RES_0_B, 8 }; cb_instructions[0x81] = { "RES 0, C", &CPU::OP_CB_RES_0_C, 8 };
    cb_instructions[0x82] = { "RES 0, D", &CPU::OP_CB_RES_0_D, 8 }; cb_instructions[0x83] = { "RES 0, E", &CPU::OP_CB_RES_0_E, 8 };
    cb_instructions[0x84] = { "RES 0, H", &CPU::OP_CB_RES_0_H, 8 }; cb_instructions[0x85] = { "RES 0, L", &CPU::OP_CB_RES_0_L, 8 };
    cb_instructions[0x86] = { "RES 0, (HL)", &CPU::OP_CB_RES_0_aHL, 16 }; cb_instructions[0x87] = { "RES 0, A", &CPU::OP_CB_RES_0_A, 8 };

    // --- Fila CB 0x88 (Bit 1) ---
    cb_instructions[0x88] = { "RES 1, B", &CPU::OP_CB_RES_1_B, 8 }; cb_instructions[0x89] = { "RES 1, C", &CPU::OP_CB_RES_1_C, 8 };
    cb_instructions[0x8A] = { "RES 1, D", &CPU::OP_CB_RES_1_D, 8 }; cb_instructions[0x8B] = { "RES 1, E", &CPU::OP_CB_RES_1_E, 8 };
    cb_instructions[0x8C] = { "RES 1, H", &CPU::OP_CB_RES_1_H, 8 }; cb_instructions[0x8D] = { "RES 1, L", &CPU::OP_CB_RES_1_L, 8 };
    cb_instructions[0x8E] = { "RES 1, (HL)", &CPU::OP_CB_RES_1_aHL, 16 }; cb_instructions[0x8F] = { "RES 1, A", &CPU::OP_CB_RES_1_A, 8 };

    // --- Fila CB 0x90 (Bit 2) ---
    cb_instructions[0x90] = { "RES 2, B", &CPU::OP_CB_RES_2_B, 8 }; cb_instructions[0x91] = { "RES 2, C", &CPU::OP_CB_RES_2_C, 8 };
    cb_instructions[0x92] = { "RES 2, D", &CPU::OP_CB_RES_2_D, 8 }; cb_instructions[0x93] = { "RES 2, E", &CPU::OP_CB_RES_2_E, 8 };
    cb_instructions[0x94] = { "RES 2, H", &CPU::OP_CB_RES_2_H, 8 }; cb_instructions[0x95] = { "RES 2, L", &CPU::OP_CB_RES_2_L, 8 };
    cb_instructions[0x96] = { "RES 2, (HL)", &CPU::OP_CB_RES_2_aHL, 16 }; cb_instructions[0x97] = { "RES 2, A", &CPU::OP_CB_RES_2_A, 8 };

    // --- Fila CB 0x98 (Bit 3) ---
    cb_instructions[0x98] = { "RES 3, B", &CPU::OP_CB_RES_3_B, 8 }; cb_instructions[0x99] = { "RES 3, C", &CPU::OP_CB_RES_3_C, 8 };
    cb_instructions[0x9A] = { "RES 3, D", &CPU::OP_CB_RES_3_D, 8 }; cb_instructions[0x9B] = { "RES 3, E", &CPU::OP_CB_RES_3_E, 8 };
    cb_instructions[0x9C] = { "RES 3, H", &CPU::OP_CB_RES_3_H, 8 }; cb_instructions[0x9D] = { "RES 3, L", &CPU::OP_CB_RES_3_L, 8 };
    cb_instructions[0x9E] = { "RES 3, (HL)", &CPU::OP_CB_RES_3_aHL, 16 }; cb_instructions[0x9F] = { "RES 3, A", &CPU::OP_CB_RES_3_A, 8 };

    // --- Fila CB 0xA0 (Bit 4) ---
    cb_instructions[0xA0] = { "RES 4, B", &CPU::OP_CB_RES_4_B, 8 }; cb_instructions[0xA1] = { "RES 4, C", &CPU::OP_CB_RES_4_C, 8 };
    cb_instructions[0xA2] = { "RES 4, D", &CPU::OP_CB_RES_4_D, 8 }; cb_instructions[0xA3] = { "RES 4, E", &CPU::OP_CB_RES_4_E, 8 };
    cb_instructions[0xA4] = { "RES 4, H", &CPU::OP_CB_RES_4_H, 8 }; cb_instructions[0xA5] = { "RES 4, L", &CPU::OP_CB_RES_4_L, 8 };
    cb_instructions[0xA6] = { "RES 4, (HL)", &CPU::OP_CB_RES_4_aHL, 16 }; cb_instructions[0xA7] = { "RES 4, A", &CPU::OP_CB_RES_4_A, 8 };

    // --- Fila CB 0xA8 (Bit 5) ---
    cb_instructions[0xA8] = { "RES 5, B", &CPU::OP_CB_RES_5_B, 8 }; cb_instructions[0xA9] = { "RES 5, C", &CPU::OP_CB_RES_5_C, 8 };
    cb_instructions[0xAA] = { "RES 5, D", &CPU::OP_CB_RES_5_D, 8 }; cb_instructions[0xAB] = { "RES 5, E", &CPU::OP_CB_RES_5_E, 8 };
    cb_instructions[0xAC] = { "RES 5, H", &CPU::OP_CB_RES_5_H, 8 }; cb_instructions[0xAD] = { "RES 5, L", &CPU::OP_CB_RES_5_L, 8 };
    cb_instructions[0xAE] = { "RES 5, (HL)", &CPU::OP_CB_RES_5_aHL, 16 }; cb_instructions[0xAF] = { "RES 5, A", &CPU::OP_CB_RES_5_A, 8 };

    // --- Fila CB 0xB0 (Bit 6) ---
    cb_instructions[0xB0] = { "RES 6, B", &CPU::OP_CB_RES_6_B, 8 }; cb_instructions[0xB1] = { "RES 6, C", &CPU::OP_CB_RES_6_C, 8 };
    cb_instructions[0xB2] = { "RES 6, D", &CPU::OP_CB_RES_6_D, 8 }; cb_instructions[0xB3] = { "RES 6, E", &CPU::OP_CB_RES_6_E, 8 };
    cb_instructions[0xB4] = { "RES 6, H", &CPU::OP_CB_RES_6_H, 8 }; cb_instructions[0xB5] = { "RES 6, L", &CPU::OP_CB_RES_6_L, 8 };
    cb_instructions[0xB6] = { "RES 6, (HL)", &CPU::OP_CB_RES_6_aHL, 16 }; cb_instructions[0xB7] = { "RES 6, A", &CPU::OP_CB_RES_6_A, 8 };

    // --- Fila CB 0xB8 (Bit 7) ---
    cb_instructions[0xB8] = { "RES 7, B", &CPU::OP_CB_RES_7_B, 8 }; cb_instructions[0xB9] = { "RES 7, C", &CPU::OP_CB_RES_7_C, 8 };
    cb_instructions[0xBA] = { "RES 7, D", &CPU::OP_CB_RES_7_D, 8 }; cb_instructions[0xBB] = { "RES 7, E", &CPU::OP_CB_RES_7_E, 8 };
    cb_instructions[0xBC] = { "RES 7, H", &CPU::OP_CB_RES_7_H, 8 }; cb_instructions[0xBD] = { "RES 7, L", &CPU::OP_CB_RES_7_L, 8 };
    cb_instructions[0xBE] = { "RES 7, (HL)", &CPU::OP_CB_RES_7_aHL, 16 }; cb_instructions[0xBF] = { "RES 7, A", &CPU::OP_CB_RES_7_A, 8 };

    // --- Fila CB 0xC0 (Bit 0) ---
    cb_instructions[0xC0] = { "SET 0, B", &CPU::OP_CB_SET_0_B, 8 }; cb_instructions[0xC1] = { "SET 0, C", &CPU::OP_CB_SET_0_C, 8 };
    cb_instructions[0xC2] = { "SET 0, D", &CPU::OP_CB_SET_0_D, 8 }; cb_instructions[0xC3] = { "SET 0, E", &CPU::OP_CB_SET_0_E, 8 };
    cb_instructions[0xC4] = { "SET 0, H", &CPU::OP_CB_SET_0_H, 8 }; cb_instructions[0xC5] = { "SET 0, L", &CPU::OP_CB_SET_0_L, 8 };
    cb_instructions[0xC6] = { "SET 0, (HL)", &CPU::OP_CB_SET_0_aHL, 16 }; cb_instructions[0xC7] = { "SET 0, A", &CPU::OP_CB_SET_0_A, 8 };

    // --- Fila CB 0xC8 (Bit 1) ---
    cb_instructions[0xC8] = { "SET 1, B", &CPU::OP_CB_SET_1_B, 8 }; cb_instructions[0xC9] = { "SET 1, C", &CPU::OP_CB_SET_1_C, 8 };
    cb_instructions[0xCA] = { "SET 1, D", &CPU::OP_CB_SET_1_D, 8 }; cb_instructions[0xCB] = { "SET 1, E", &CPU::OP_CB_SET_1_E, 8 };
    cb_instructions[0xCC] = { "SET 1, H", &CPU::OP_CB_SET_1_H, 8 }; cb_instructions[0xCD] = { "SET 1, L", &CPU::OP_CB_SET_1_L, 8 };
    cb_instructions[0xCE] = { "SET 1, (HL)", &CPU::OP_CB_SET_1_aHL, 16 }; cb_instructions[0xCF] = { "SET 1, A", &CPU::OP_CB_SET_1_A, 8 };

    // --- Fila CB 0xD0 (Bit 2) ---
    cb_instructions[0xD0] = { "SET 2, B", &CPU::OP_CB_SET_2_B, 8 }; cb_instructions[0xD1] = { "SET 2, C", &CPU::OP_CB_SET_2_C, 8 };
    cb_instructions[0xD2] = { "SET 2, D", &CPU::OP_CB_SET_2_D, 8 }; cb_instructions[0xD3] = { "SET 2, E", &CPU::OP_CB_SET_2_E, 8 };
    cb_instructions[0xD4] = { "SET 2, H", &CPU::OP_CB_SET_2_H, 8 }; cb_instructions[0xD5] = { "SET 2, L", &CPU::OP_CB_SET_2_L, 8 };
    cb_instructions[0xD6] = { "SET 2, (HL)", &CPU::OP_CB_SET_2_aHL, 16 }; cb_instructions[0xD7] = { "SET 2, A", &CPU::OP_CB_SET_2_A, 8 };

    // --- Fila CB 0xD8 (Bit 3) ---
    cb_instructions[0xD8] = { "SET 3, B", &CPU::OP_CB_SET_3_B, 8 }; cb_instructions[0xD9] = { "SET 3, C", &CPU::OP_CB_SET_3_C, 8 };
    cb_instructions[0xDA] = { "SET 3, D", &CPU::OP_CB_SET_3_D, 8 }; cb_instructions[0xDB] = { "SET 3, E", &CPU::OP_CB_SET_3_E, 8 };
    cb_instructions[0xDC] = { "SET 3, H", &CPU::OP_CB_SET_3_H, 8 }; cb_instructions[0xDD] = { "SET 3, L", &CPU::OP_CB_SET_3_L, 8 };
    cb_instructions[0xDE] = { "SET 3, (HL)", &CPU::OP_CB_SET_3_aHL, 16 }; cb_instructions[0xDF] = { "SET 3, A", &CPU::OP_CB_SET_3_A, 8 };

    // --- Fila CB 0xE0 (Bit 4) ---
    cb_instructions[0xE0] = { "SET 4, B", &CPU::OP_CB_SET_4_B, 8 }; cb_instructions[0xE1] = { "SET 4, C", &CPU::OP_CB_SET_4_C, 8 };
    cb_instructions[0xE2] = { "SET 4, D", &CPU::OP_CB_SET_4_D, 8 }; cb_instructions[0xE3] = { "SET 4, E", &CPU::OP_CB_SET_4_E, 8 };
    cb_instructions[0xE4] = { "SET 4, H", &CPU::OP_CB_SET_4_H, 8 }; cb_instructions[0xE5] = { "SET 4, L", &CPU::OP_CB_SET_4_L, 8 };
    cb_instructions[0xE6] = { "SET 4, (HL)", &CPU::OP_CB_SET_4_aHL, 16 }; cb_instructions[0xE7] = { "SET 4, A", &CPU::OP_CB_SET_4_A, 8 };

    // --- Fila CB 0xE8 (Bit 5) ---
    cb_instructions[0xE8] = { "SET 5, B", &CPU::OP_CB_SET_5_B, 8 }; cb_instructions[0xE9] = { "SET 5, C", &CPU::OP_CB_SET_5_C, 8 };
    cb_instructions[0xEA] = { "SET 5, D", &CPU::OP_CB_SET_5_D, 8 }; cb_instructions[0xEB] = { "SET 5, E", &CPU::OP_CB_SET_5_E, 8 };
    cb_instructions[0xEC] = { "SET 5, H", &CPU::OP_CB_SET_5_H, 8 }; cb_instructions[0xED] = { "SET 5, L", &CPU::OP_CB_SET_5_L, 8 };
    cb_instructions[0xEE] = { "SET 5, (HL)", &CPU::OP_CB_SET_5_aHL, 16 }; cb_instructions[0xEF] = { "SET 5, A", &CPU::OP_CB_SET_5_A, 8 };

    // --- Fila CB 0xF0 (Bit 6) ---
    cb_instructions[0xF0] = { "SET 6, B", &CPU::OP_CB_SET_6_B, 8 }; cb_instructions[0xF1] = { "SET 6, C", &CPU::OP_CB_SET_6_C, 8 };
    cb_instructions[0xF2] = { "SET 6, D", &CPU::OP_CB_SET_6_D, 8 }; cb_instructions[0xF3] = { "SET 6, E", &CPU::OP_CB_SET_6_E, 8 };
    cb_instructions[0xF4] = { "SET 6, H", &CPU::OP_CB_SET_6_H, 8 }; cb_instructions[0xF5] = { "SET 6, L", &CPU::OP_CB_SET_6_L, 8 };
    cb_instructions[0xF6] = { "SET 6, (HL)", &CPU::OP_CB_SET_6_aHL, 16 }; cb_instructions[0xF7] = { "SET 6, A", &CPU::OP_CB_SET_6_A, 8 };

    // --- Fila CB 0xF8 (Bit 7) ---
    cb_instructions[0xF8] = { "SET 7, B", &CPU::OP_CB_SET_7_B, 8 }; cb_instructions[0xF9] = { "SET 7, C", &CPU::OP_CB_SET_7_C, 8 };
    cb_instructions[0xFA] = { "SET 7, D", &CPU::OP_CB_SET_7_D, 8 }; cb_instructions[0xFB] = { "SET 7, E", &CPU::OP_CB_SET_7_E, 8 };
    cb_instructions[0xFC] = { "SET 7, H", &CPU::OP_CB_SET_7_H, 8 }; cb_instructions[0xFD] = { "SET 7, L", &CPU::OP_CB_SET_7_L, 8 };
    cb_instructions[0xFE] = { "SET 7, (HL)", &CPU::OP_CB_SET_7_aHL, 16 }; cb_instructions[0xFF] = { "SET 7, A", &CPU::OP_CB_SET_7_A, 8 };

}

void CPU::tick() {
    cycles += 4;
    bus.ppu.step(4);
    bus.apu.step(4);
    bus.updateTimers(4);
}

Byte CPU::read(Word address) {
    tick();
    return bus.read(address);
}

void CPU::write(Word address, Byte value) {
    tick();
    bus.write(address, value);
}

void CPU::step() {
    if (isStopped) {
        tick();
        return;
    }
    if (isHalted) {
        tick();
        return;
    }

    if (imeDelay > 0) {
        imeDelay--;
        if (imeDelay == 0) {
            ime = true;
        }
    }
    // Fetch
    Byte opcode = fetchByte();

    // Decode & Execute
    Instruction inst = instructions[opcode];
    
    (this->*inst.operate)();   
}

void CPU::reset() {
    PC = 0x0100; // Punto de entrada estándar de la GB
    // TODO: valores por defecto de todos los registros.
        // Configuración post-BIOS
    A = 0x01; F = 0xB0;
    B = 0x00; C = 0x13;
    D = 0x00; E = 0xD8;
    H = 0x01; L = 0x4D;
    SP = 0xFFFE;
}

// FETCH
Byte CPU::fetchByte() {
    Byte data = read(PC);
    PC++;
    return data;
}

Word CPU::fetchWord() {
    Byte data1 = read(PC);
    PC++;
    Byte data2 = read(PC);
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

void CPU::bit(Byte regVal, int bitIndex) {
    bool isZero = !((regVal >> bitIndex) & 1);
    setFlag(F_Z, isZero);

    // Flags fijos según manual
    setFlag(F_N, false);
    setFlag(F_H, true); // BIT siempre enciende H
    // C no se toca
}

// Apagar un bit
void CPU::res(int bitIndex, Byte& regVal) {
    regVal &= ~(1 << bitIndex);
}

// Encender un bit
void CPU::set(int bitIndex, Byte& regVal) {
    regVal |= (1 << bitIndex);
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
    tick();

    // 1. High Byte
    SP--;
    write(SP, (value >> 8) & 0xFF);

    // 2. Low Byte
    SP--;
    write(SP, value & 0xFF);
}

Word CPU::popStack() {
    // 1. Leemos el Byte BAJO primero (porque el Stack es LIFO - Last In First Out)
    Byte lo = read(SP);
    SP++;

    // 2. Leemos el Byte ALTO
    Byte hi = read(SP);
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

// Shift left arithmetic
void CPU::sla(Byte &reg) {
    const bool isCarry = (reg & 0x80) != 0;

    reg <<= 1;

    setFlag(F_Z, reg == 0);
    setFlag(F_N, false);    
    setFlag(F_H, false);    
    setFlag(F_C, isCarry);        
}

// Shift right arithmetic
void CPU::sra(Byte &reg) {
    const bool isCarry = (reg & 0x01) != 0;
    const Byte sign = (reg & 0x80);

    reg = (reg >> 1) | sign;

    setFlag(F_Z, reg == 0);
    setFlag(F_N, false);    
    setFlag(F_H, false);
    setFlag(F_C, isCarry);        
}

// Swap
void CPU::swap(Byte &reg) {
    reg = (reg << 4) | (reg >> 4);

    setFlag(F_Z, reg == 0);
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, false);    
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
        isHalted = false;
    }

    // Si las interrupciones están apagadas, no hacemos nada
    if (!ime) return;

    // Miramos si hay alguna interrupción activa QUE ADEMÁS esté habilitada
    // (Ej: Si VBlank (bit 0) está en 1 en ambos registros)
    if (IE & IF & 0x1F) { // 0x1F son los 5 bits de interrupciones válidos
        ime = false;

        tick();
        tick();
        tick();

        // Identificamos cual fue (VBlank es la más prioritaria, bit 0)
        Byte interruptMask = IE & IF;

        Word vector = 0x0000;

        // Bit 0: VBlank (INT 40h)
        if (interruptMask & 0x01) {
            bus.write(0xFF0F, bus.read(0xFF0F) & ~0x01); // Limpiamos el flag (ack)
            vector = 0x0040;
        }
        // Bit 1: LCD STAT (INT 48h)
        else if (interruptMask & 0x02) {
            bus.write(0xFF0F, bus.read(0xFF0F) & ~0x02);
            vector = 0x0048;
        }
        // Bit 2: Timer (INT 50h)
        else if (interruptMask & 0x04) {
            bus.write(0xFF0F, bus.read(0xFF0F) & ~0x04);
            vector = 0x0050;
        }
        // Bit 3: Serial (INT 58h)
        else if (interruptMask & 0x08) {
            bus.write(0xFF0F, bus.read(0xFF0F) & ~0x08);
            vector = 0x0058;
        }
        // Bit 4: Joypad (INT 60h)
        else if (interruptMask & 0x10) {
            bus.write(0xFF0F, bus.read(0xFF0F) & ~0x10);
            vector = 0x0060;
        }

        // Saltar
        pushStack(PC);
        PC = vector;
    }
}

// ========= Nueva tabla ========= //

void CPU::OP_UNKNOWN() {
    // Recolectar el op error
    PC--; 
    Byte illegalOpcode = bus.read(PC);
    
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Opcode Ilegal o Desconocido: 0x%02X en PC: 0x%04X", illegalOpcode, PC);
    
    throw std::runtime_error(buffer);
}

// =========================================================
// Opcodes 0x00 - 0x0F
// =========================================================
void CPU::OP_NOP() {}
void CPU::OP_LD_BC_d16() { setBC(fetchWord()); }
void CPU::OP_LD_BC_A()   { write(getBC(), A);}
void CPU::OP_INC_BC()    { tick(); setBC(getBC() + 1);}
void CPU::OP_INC_B()     { inc(B); }
void CPU::OP_DEC_B()     { dec(B); }
void CPU::OP_LD_B_d8()   { B = fetchByte(); }
void CPU::OP_RLCA()      { rlc(A, false); }
void CPU::OP_LD_a16_SP() {
    Word addr = fetchWord();
    write(addr, SP);
    write(addr + 1, (SP >> 8));
}
void CPU::OP_ADD_HL_BC() { tick(); addHL(getBC()); }
void CPU::OP_LD_A_BC()   { A = read(getBC()); }
void CPU::OP_DEC_BC()    { tick(); setBC(getBC() - 1); }
void CPU::OP_INC_C()     { inc(C); }
void CPU::OP_DEC_C()     { dec(C); }
void CPU::OP_LD_C_d8()   { C = fetchByte(); }
void CPU::OP_RRCA()      { rrc(A, false); }

// =========================================================
// Opcodes 0x10 - 0x1F
// =========================================================
void CPU::OP_STOP()      {
    //fetchByte();
    PC++;
    isStopped = true;
}
void CPU::OP_LD_DE_d16() { setDE(fetchWord()); }
void CPU::OP_LD_DE_A()   { write(getDE(), A); }
void CPU::OP_INC_DE()    { tick(); setDE(getDE() + 1); }
void CPU::OP_INC_D()     { inc(D); }
void CPU::OP_DEC_D()     { dec(D); }
void CPU::OP_LD_D_d8()   { D = fetchByte(); }
void CPU::OP_RLA()       {
    rl(A);
    setFlag(F_Z, false);
}
void CPU::OP_JR_r8()     {
    tick();
    const auto offset = static_cast<int8_t>(fetchByte());
    PC += offset;
}
void CPU::OP_ADD_HL_DE() { tick(); addHL(getDE()); }
void CPU::OP_LD_A_DE()   { A = read(getDE()); }
void CPU::OP_DEC_DE()    { tick(); setDE(getDE() - 1); }
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
        tick();
    }
}
void CPU::OP_LD_HL_d16() { setHL(fetchWord()); }
void CPU::OP_LDI_HL_A() {
    write(getHL(), A);
    setHL(getHL() + 1);
}
void CPU::OP_INC_HL()    { tick(); setHL(getHL() + 1); }
void CPU::OP_INC_H()     { inc(H); }
void CPU::OP_DEC_H()     { dec(H); }
void CPU::OP_LD_H_d8()   { H = fetchByte(); }
void CPU::OP_DAA()       { daa(); }
void CPU::OP_JR_Z_r8() {
    const auto offset = static_cast<int8_t>(fetchByte());

    if (getFlag(F_Z)) {
        PC += offset;
        tick();
    }
}
void CPU::OP_ADD_HL_HL() { tick(); addHL(getHL()); }
void CPU::OP_LDI_A_HL()  {
    A = read(getHL());
    setHL(getHL() + 1); // HL++
}
void CPU::OP_DEC_HL()    { tick(); setHL(getHL() - 1); }
void CPU::OP_INC_L()     { inc(L);}
void CPU::OP_DEC_L()     { dec(L); }
void CPU::OP_LD_L_d8()   { L = fetchByte(); }
void CPU::OP_CPL() {
    A = ~A;
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
        tick();
    }
}
void CPU::OP_LD_SP_d16() { SP = fetchWord(); }
void CPU::OP_LDD_HL_A() {
    write(getHL(), A);
    setHL(getHL() - 1);
}
void CPU::OP_INC_SP()    { tick(); SP++; }
void CPU::OP_INC_aHL() {
    Byte val = read(getHL());
    inc(val);
    write(getHL(), val);
}
void CPU::OP_DEC_aHL() {
    Byte val = read(getHL());
    dec(val);
    write(getHL(), val);
}
void CPU::OP_LD_aHL_d8() {
    const Byte val = fetchByte();
    write(getHL(), val);
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
        tick();
    }
}
void CPU::OP_ADD_HL_SP() { tick(); addHL(SP); }
void CPU::OP_LDD_A_HL() {
    A = read(getHL());
    setHL(getHL() - 1);
}
void CPU::OP_DEC_SP()    { tick(); SP--; }
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
void CPU::OP_LD_B_aHL(){B = read(getHL());}
void CPU::OP_LD_B_A() {B = A;}
void CPU::OP_LD_C_B() {C = B;}
void CPU::OP_LD_C_C() {C = C;}
void CPU::OP_LD_C_D() {C = D;}
void CPU::OP_LD_C_E() {C = E;}
void CPU::OP_LD_C_H() {C = H;}
void CPU::OP_LD_C_L() {C = L;}
void CPU::OP_LD_C_aHL(){C = read(getHL());}
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
void CPU::OP_LD_D_aHL() {D = read(getHL());}
void CPU::OP_LD_D_A() {D = A;}
void CPU::OP_LD_E_B() {E = B;}
void CPU::OP_LD_E_C() {E = C;}
void CPU::OP_LD_E_D() {E = D;}
void CPU::OP_LD_E_E() {E = E;}
void CPU::OP_LD_E_H() {E = H;}
void CPU::OP_LD_E_L() {E = L;}
void CPU::OP_LD_E_aHL() { E = read(getHL());}
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
void CPU::OP_LD_H_aHL() { H = read(getHL()); }
void CPU::OP_LD_H_A()   { H = A; }
void CPU::OP_LD_L_B()   { L = B; }
void CPU::OP_LD_L_C()   { L = C; }
void CPU::OP_LD_L_D()   { L = D; }
void CPU::OP_LD_L_E()   { L = E; }
void CPU::OP_LD_L_H()   { L = H; }
void CPU::OP_LD_L_L()   { L = L; }
void CPU::OP_LD_L_aHL() { L = read(getHL()); }
void CPU::OP_LD_L_A()   { L = A; }

// =========================================================
// Opcodes 0x70 - 0x7F
// =========================================================
void CPU::OP_LD_aHL_B() { write(getHL(), B); }
void CPU::OP_LD_aHL_C() { write(getHL(), C);}
void CPU::OP_LD_aHL_D() { write(getHL(), D);}
void CPU::OP_LD_aHL_E() { write(getHL(), E);}
void CPU::OP_LD_aHL_H() { write(getHL(), H);}
void CPU::OP_LD_aHL_L() { write(getHL(), L);}
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
void CPU::OP_LD_aHL_A() { write(getHL(), A); }
void CPU::OP_LD_A_B()   { A = B; }
void CPU::OP_LD_A_C()   { A = C; }
void CPU::OP_LD_A_D()   { A = D; }
void CPU::OP_LD_A_E()   { A = E; }
void CPU::OP_LD_A_H()   { A = H; }
void CPU::OP_LD_A_L()   { A = L; }
void CPU::OP_LD_A_aHL() { A = read(getHL());}
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
    const Byte val = read(getHL());
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
    const Byte val = read(getHL());
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
    const Byte val = read(getHL());
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
    const Byte val = read(getHL());
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
void CPU::OP_AND_aHL() { and_op(read(getHL()));}
void CPU::OP_AND_A()   { and_op(A);}
void CPU::OP_XOR_B()   { xor_op(B);}
void CPU::OP_XOR_C()   { xor_op(C); }
void CPU::OP_XOR_D()   { xor_op(D); }
void CPU::OP_XOR_E()   { xor_op(E); }
void CPU::OP_XOR_H()   { xor_op(H); }
void CPU::OP_XOR_L()   { xor_op(L); }
void CPU::OP_XOR_aHL() { xor_op(read(getHL())); }
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
void CPU::OP_OR_aHL()  { or_op(read(getHL())); }
void CPU::OP_OR_A()    { or_op(A); }
void CPU::OP_CP_B()    { cp(B);}
void CPU::OP_CP_C()    { cp(C); }
void CPU::OP_CP_D()    { cp(D); }
void CPU::OP_CP_E()    { cp(E); }
void CPU::OP_CP_H()    { cp(H); }
void CPU::OP_CP_L()    { cp(L); }
void CPU::OP_CP_aHL()  { cp(read(getHL())); }
void CPU::OP_CP_A()    { cp(A); }

// =========================================================
// Opcodes 0xC0 - 0xCF
// =========================================================
void CPU::OP_RET_NZ() {
    tick();
    if (!getFlag(F_Z)) {
        tick();
        PC = popStack();
    }
}
void CPU::OP_POP_BC()      { setBC(popStack());}
void CPU::OP_JP_NZ_a16() {
    Word target = fetchWord();
    if (!getFlag(F_Z)) {
        PC = target;
        tick();
    }
}
void CPU::OP_JP_a16() {
    tick();
    Word targetAddress = fetchWord();
    PC = targetAddress;
}
void CPU::OP_CALL_NZ_a16() {
    Word target = fetchWord();
    if (!getFlag(F_Z)) {
        pushStack(PC);
        PC = target;
    }
}
void CPU::OP_PUSH_BC()     { pushStack(getBC());}
void CPU::OP_ADD_A_d8()    { add(fetchByte()); }
void CPU::OP_RST_00H() {
    pushStack(PC);
    PC = 0x0000;
}
void CPU::OP_RET_Z() {
    tick();
    if (getFlag(F_Z)) {
        tick();
        PC = popStack();
    }      
}
void CPU::OP_RET()         { tick(); PC = popStack();}
void CPU::OP_JP_Z_a16() {
    Word target = fetchWord();
    if (getFlag(F_Z)) {
        tick();
        PC = target;
    }
}

void CPU::OP_CALL_Z_a16() { 
    Word target = fetchWord();
    if (getFlag(F_Z)) {
        pushStack(PC);
        PC = target;
    }
}
void CPU::OP_CALL_a16() {
    Word targetAddr = fetchWord();
    pushStack(PC);
    PC = targetAddr;
}
void CPU::OP_ADC_A_d8()    { adc(fetchByte()); }
void CPU::OP_RST_08H() {
    pushStack(PC);
    PC = 0x0008;
}

// =========================================================
// Opcodes 0xD0 - 0xDF
// =========================================================

void CPU::OP_RET_NC() {
    tick();
    if (!getFlag(F_C)) {
        tick();
        PC = popStack();
    }
}
void CPU::OP_POP_DE()      {setDE(popStack());}
void CPU::OP_JP_NC_a16() {
    Word target = fetchWord();
    if (!getFlag(F_C)) {
        tick();
        PC = target;
    }
}
void CPU::OP_CALL_NC_a16() {
    Word target = fetchWord();
    if (!getFlag(F_C)) {
        pushStack(PC);
        PC = target;
    }
}
void CPU::OP_PUSH_DE()     { pushStack(getDE());}
void CPU::OP_SUB_d8()      { sub(fetchByte());}
void CPU::OP_RST_10H() {
    pushStack(PC);
    PC = 0x0010;
}
void CPU::OP_RET_C() {
    tick();
    if (getFlag(F_C)) {
        tick();
        PC = popStack();
    }
}
void CPU::OP_RETI() {
    tick();
    // Regresamos de donde vinimos (igual que RET)
    PC = popStack();
    // Volvemos a encender las interrupciones
    ime = true;
}
void CPU::OP_JP_C_a16() {
    Word target = fetchWord();
    if (getFlag(F_C)) {
        tick();
        PC = target;
    }
}
void CPU::OP_CALL_C_a16() {
    Word target = fetchWord();
    if (getFlag(F_C)) {
        pushStack(PC);
        PC = target;
    }
}
void CPU::OP_SBC_A_d8()    { sbc(fetchByte()); }
void CPU::OP_RST_18H() {
    pushStack(PC);
    PC = 0x0018;
}

// =========================================================
// Opcodes 0xE0 - 0xEF
// =========================================================

void CPU::OP_LDH_a8_A()  { write(0xFF00 + fetchByte(), A); }
void CPU::OP_POP_HL()    { setHL(popStack()); }
void CPU::OP_LD_C_A_BUS()    { write(0xFF00 + C, A); }
void CPU::OP_PUSH_HL()   { pushStack(getHL()); }
void CPU::OP_AND_d8()    { and_op(fetchByte()); }
void CPU::OP_RST_20H()   { pushStack(PC); PC = 0x0020; }
void CPU::OP_ADD_SP_r8() {
    auto offset = static_cast<int8_t>(fetchByte());

    // Calculamos los flags usando la versión SIN SIGNO del offset (uint8_t)
    // y solo la parte baja (0xFF) de SP.
    int result = (SP & 0xFF) + static_cast<uint8_t>(offset);

    setFlag(F_Z, false);
    setFlag(F_N, false);
    setFlag(F_H, ((SP & 0x0F) + (static_cast<uint8_t>(offset) & 0x0F)) > 0x0F);
    setFlag(F_C, result > 0xFF);

    // Realizamos la suma real (SP es de 16 bits)
    tick(); tick();
    SP += offset;
}
void CPU::OP_JP_HL()     { PC = getHL(); /*Salta a la direccion de HL*/ }
void CPU::OP_LD_a16_A()  { write(fetchWord(), A);}
void CPU::OP_XOR_d8()    { xor_op(fetchByte());}
void CPU::OP_RST_28H()   { pushStack(PC); PC = 0x0028; }

// =========================================================
// Opcodes 0xF0 - 0xFF
// =========================================================
void CPU::OP_LDH_A_a8()  { A = read(0xFF00 + fetchByte()); }
void CPU::OP_POP_AF()    { setAF(popStack()); }
void CPU::OP_LD_A_C_BUS()    { A = read(0xFF00 + C); }
void CPU::OP_DI() {
    ime = false;
    imeDelay = 0;
}
void CPU::OP_PUSH_AF()   { pushStack(getAF()); }
void CPU::OP_OR_d8()     { or_op(fetchByte()); }
void CPU::OP_RST_30H()   { pushStack(PC); PC = 0x0030; }
void CPU::OP_LD_HL_SP_r8() {
    auto offset = static_cast<int8_t>(fetchByte());
    int result = (SP & 0xFF) + static_cast<uint8_t>(offset);

    setFlag(F_Z, false);
    setFlag(F_N, false);
    setFlag(F_H, ((SP & 0x0F) + (static_cast<uint8_t>(offset) & 0x0F)) > 0x0F);
    setFlag(F_C, result > 0xFF);
    
    tick();

    setHL(SP + offset);
}
void CPU::OP_LD_SP_HL()  { tick(); SP = getHL(); }
void CPU::OP_LD_A_a16()  { A = read(fetchWord()); }
void CPU::OP_EI(){
    //ime = true;
    imeDelay = 2;
}
void CPU::OP_CP_d8()     { cp(fetchByte()); }
void CPU::OP_RST_38H()   { pushStack(PC); PC = 0x0038; }



// ======================== FUNCIONES PREFIX CB =================================//

void CPU::OP_UNKNOWN_CB() {
    PC -= 2; 
    Byte cbOpcode = bus.read(PC + 1);
    
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Opcode CB no mapeado en el emulador: CB %02X en PC: 0x%04X", cbOpcode, PC);
    
    throw std::runtime_error(buffer);
}

void CPU::OP_PREFIX_CB() {
    const Byte& cb_opcode = fetchByte();
    Instruction inst = cb_instructions[cb_opcode];
    //if (inst.cycles > 0) {
    //    cycles += (inst.cycles - 4); 
    //}
    (this->*inst.operate)();
}

// =========================================================
// Opcodes CB: 0x00 - 0x0F
// =========================================================
void CPU::OP_CB_RLC_B()   { rlc(B, true); }
void CPU::OP_CB_RLC_C()   { rlc(C, true); }
void CPU::OP_CB_RLC_D()   { rlc(D, true); }
void CPU::OP_CB_RLC_E()   { rlc(E, true); }
void CPU::OP_CB_RLC_H()   { rlc(H, true); }
void CPU::OP_CB_RLC_L()   { rlc(L, true); }
void CPU::OP_CB_RLC_aHL() { 
    // Para HL hay que leer y luego escribir de vuelta
    Byte val = read(getHL());
    rlc(val, true);
    write(getHL(), val);
}
void CPU::OP_CB_RLC_A()   { rlc(A, true); }
void CPU::OP_CB_RRC_B()   { rrc(B, true); }
void CPU::OP_CB_RRC_C()   { rrc(C, true); }
void CPU::OP_CB_RRC_D()   { rrc(D, true); }
void CPU::OP_CB_RRC_E()   { rrc(E, true); }
void CPU::OP_CB_RRC_H()   { rrc(H, true); }
void CPU::OP_CB_RRC_L()   { rrc(L, true); }
void CPU::OP_CB_RRC_aHL() { 
    Byte val = read(getHL());
    rrc(val, true);
    write(getHL(), val);
}
void CPU::OP_CB_RRC_A()   { rrc(A, true); }

// =========================================================
// Opcodes CB: 0x10 - 0x1F
// =========================================================
void CPU::OP_CB_RL_B()   { rl(B); }
void CPU::OP_CB_RL_C()   { rl(C); }
void CPU::OP_CB_RL_D()   { rl(D); }
void CPU::OP_CB_RL_E()   { rl(E); }
void CPU::OP_CB_RL_H()   { rl(H); }
void CPU::OP_CB_RL_L()   { rl(L); }
void CPU::OP_CB_RL_aHL() { 
    Byte val = read(getHL());
    rl(val);
    write(getHL(), val);
}
void CPU::OP_CB_RL_A()   { rl(A); }

void CPU::OP_CB_RR_B()   { rr(B); }
void CPU::OP_CB_RR_C()   { rr(C); }
void CPU::OP_CB_RR_D()   { rr(D); }
void CPU::OP_CB_RR_E()   { rr(E); }
void CPU::OP_CB_RR_H()   { rr(H); }
void CPU::OP_CB_RR_L()   { rr(L); }
void CPU::OP_CB_RR_aHL() { 
    Byte val = read(getHL());
    rr(val);
    write(getHL(), val);
}
void CPU::OP_CB_RR_A()   { rr(A); }

// =========================================================
// Opcodes CB: 0x20 - 0x2F
// =========================================================
void CPU::OP_CB_SLA_B()   { sla(B); }
void CPU::OP_CB_SLA_C()   { sla(C); }
void CPU::OP_CB_SLA_D()   { sla(D); }
void CPU::OP_CB_SLA_E()   { sla(E); }
void CPU::OP_CB_SLA_H()   { sla(H); }
void CPU::OP_CB_SLA_L()   { sla(L); }
void CPU::OP_CB_SLA_aHL() { 
    Byte val = read(getHL());
    sla(val);
    write(getHL(), val);
}
void CPU::OP_CB_SLA_A()   { sla(A); }

void CPU::OP_CB_SRA_B()   { sra(B); }
void CPU::OP_CB_SRA_C()   { sra(C); }
void CPU::OP_CB_SRA_D()   { sra(D); }
void CPU::OP_CB_SRA_E()   { sra(E); }
void CPU::OP_CB_SRA_H()   { sra(H); }
void CPU::OP_CB_SRA_L()   { sra(L); }
void CPU::OP_CB_SRA_aHL() { 
    Byte val = read(getHL());
    sra(val);
    write(getHL(), val);
}
void CPU::OP_CB_SRA_A()   { sra(A); }

// =========================================================
// Opcodes CB: 0x30 - 0x3F
// =========================================================
void CPU::OP_CB_SWAP_B()   { swap(B); }
void CPU::OP_CB_SWAP_C()   { swap(C); }
void CPU::OP_CB_SWAP_D()   { swap(D); }
void CPU::OP_CB_SWAP_E()   { swap(E); }
void CPU::OP_CB_SWAP_H()   { swap(H); }
void CPU::OP_CB_SWAP_L()   { swap(L); }
void CPU::OP_CB_SWAP_aHL() { 
    Byte val = read(getHL());
    swap(val);
    write(getHL(), val);
}
void CPU::OP_CB_SWAP_A()   { swap(A); }

void CPU::OP_CB_SRL_B()    { srl(B); }
void CPU::OP_CB_SRL_C()    { srl(C); }
void CPU::OP_CB_SRL_D()    { srl(D); }
void CPU::OP_CB_SRL_E()    { srl(E); }
void CPU::OP_CB_SRL_H()    { srl(H); }
void CPU::OP_CB_SRL_L()    { srl(L); }
void CPU::OP_CB_SRL_aHL()  { 
    Byte val = read(getHL());
    srl(val);
    write(getHL(), val);
}
void CPU::OP_CB_SRL_A()    { srl(A); }

// =========================================================
// Opcodes CB: 0x40 - 0x4F (Bits 0 y 1)
// =========================================================
void CPU::OP_CB_BIT_0_B() { bit(B, 0); } void CPU::OP_CB_BIT_0_C() { bit(C, 0); }
void CPU::OP_CB_BIT_0_D() { bit(D, 0); } void CPU::OP_CB_BIT_0_E() { bit(E, 0); }
void CPU::OP_CB_BIT_0_H() { bit(H, 0); } void CPU::OP_CB_BIT_0_L() { bit(L, 0); }
void CPU::OP_CB_BIT_0_aHL() { bit(read(getHL()), 0); } void CPU::OP_CB_BIT_0_A() { bit(A, 0); }

void CPU::OP_CB_BIT_1_B() { bit(B, 1); } void CPU::OP_CB_BIT_1_C() { bit(C, 1); }
void CPU::OP_CB_BIT_1_D() { bit(D, 1); } void CPU::OP_CB_BIT_1_E() { bit(E, 1); }
void CPU::OP_CB_BIT_1_H() { bit(H, 1); } void CPU::OP_CB_BIT_1_L() { bit(L, 1); }
void CPU::OP_CB_BIT_1_aHL() { bit(read(getHL()), 1); } void CPU::OP_CB_BIT_1_A() { bit(A, 1); }

// =========================================================
// Opcodes CB: 0x50 - 0x5F (Bits 2 y 3)
// =========================================================
void CPU::OP_CB_BIT_2_B() { bit(B, 2); } void CPU::OP_CB_BIT_2_C() { bit(C, 2); }
void CPU::OP_CB_BIT_2_D() { bit(D, 2); } void CPU::OP_CB_BIT_2_E() { bit(E, 2); }
void CPU::OP_CB_BIT_2_H() { bit(H, 2); } void CPU::OP_CB_BIT_2_L() { bit(L, 2); }
void CPU::OP_CB_BIT_2_aHL() { bit(read(getHL()), 2); } void CPU::OP_CB_BIT_2_A() { bit(A, 2); }

void CPU::OP_CB_BIT_3_B() { bit(B, 3); } void CPU::OP_CB_BIT_3_C() { bit(C, 3); }
void CPU::OP_CB_BIT_3_D() { bit(D, 3); } void CPU::OP_CB_BIT_3_E() { bit(E, 3); }
void CPU::OP_CB_BIT_3_H() { bit(H, 3); } void CPU::OP_CB_BIT_3_L() { bit(L, 3); }
void CPU::OP_CB_BIT_3_aHL() { bit(read(getHL()), 3); } void CPU::OP_CB_BIT_3_A() { bit(A, 3); }

// =========================================================
// Opcodes CB: 0x60 - 0x6F (Bits 4 y 5)
// =========================================================
void CPU::OP_CB_BIT_4_B() { bit(B, 4); } void CPU::OP_CB_BIT_4_C() { bit(C, 4); }
void CPU::OP_CB_BIT_4_D() { bit(D, 4); } void CPU::OP_CB_BIT_4_E() { bit(E, 4); }
void CPU::OP_CB_BIT_4_H() { bit(H, 4); } void CPU::OP_CB_BIT_4_L() { bit(L, 4); }
void CPU::OP_CB_BIT_4_aHL() { bit(read(getHL()), 4); } void CPU::OP_CB_BIT_4_A() { bit(A, 4); }

void CPU::OP_CB_BIT_5_B() { bit(B, 5); } void CPU::OP_CB_BIT_5_C() { bit(C, 5); }
void CPU::OP_CB_BIT_5_D() { bit(D, 5); } void CPU::OP_CB_BIT_5_E() { bit(E, 5); }
void CPU::OP_CB_BIT_5_H() { bit(H, 5); } void CPU::OP_CB_BIT_5_L() { bit(L, 5); }
void CPU::OP_CB_BIT_5_aHL() { bit(read(getHL()), 5); } void CPU::OP_CB_BIT_5_A() { bit(A, 5); }

// =========================================================
// Opcodes CB: 0x70 - 0x7F (Bits 6 y 7)
// =========================================================
void CPU::OP_CB_BIT_6_B() { bit(B, 6); } void CPU::OP_CB_BIT_6_C() { bit(C, 6); }
void CPU::OP_CB_BIT_6_D() { bit(D, 6); } void CPU::OP_CB_BIT_6_E() { bit(E, 6); }
void CPU::OP_CB_BIT_6_H() { bit(H, 6); } void CPU::OP_CB_BIT_6_L() { bit(L, 6); }
void CPU::OP_CB_BIT_6_aHL() { bit(read(getHL()), 6); } void CPU::OP_CB_BIT_6_A() { bit(A, 6); }

void CPU::OP_CB_BIT_7_B() { bit(B, 7); } void CPU::OP_CB_BIT_7_C() { bit(C, 7); }
void CPU::OP_CB_BIT_7_D() { bit(D, 7); } void CPU::OP_CB_BIT_7_E() { bit(E, 7); }
void CPU::OP_CB_BIT_7_H() { bit(H, 7); } void CPU::OP_CB_BIT_7_L() { bit(L, 7); }
void CPU::OP_CB_BIT_7_aHL() { bit(read(getHL()), 7); } void CPU::OP_CB_BIT_7_A() { bit(A, 7); }

// =========================================================
// Opcodes CB: 0x80 - 0x8F (Bits 0 y 1)
// =========================================================
void CPU::OP_CB_RES_0_B() { res(0, B); } void CPU::OP_CB_RES_0_C() { res(0, C); }
void CPU::OP_CB_RES_0_D() { res(0, D); } void CPU::OP_CB_RES_0_E() { res(0, E); }
void CPU::OP_CB_RES_0_H() { res(0, H); } void CPU::OP_CB_RES_0_L() { res(0, L); }
void CPU::OP_CB_RES_0_aHL() { Byte val = read(getHL()); res(0, val); write(getHL(), val); } void CPU::OP_CB_RES_0_A() { res(0, A); }

void CPU::OP_CB_RES_1_B() { res(1, B); } void CPU::OP_CB_RES_1_C() { res(1, C); }
void CPU::OP_CB_RES_1_D() { res(1, D); } void CPU::OP_CB_RES_1_E() { res(1, E); }
void CPU::OP_CB_RES_1_H() { res(1, H); } void CPU::OP_CB_RES_1_L() { res(1, L); }
void CPU::OP_CB_RES_1_aHL() { Byte val = read(getHL()); res(1, val); write(getHL(), val); } void CPU::OP_CB_RES_1_A() { res(1, A); }

// =========================================================
// Opcodes CB: 0x90 - 0x9F (Bits 2 y 3)
// =========================================================
void CPU::OP_CB_RES_2_B() { res(2, B); } void CPU::OP_CB_RES_2_C() { res(2, C); }
void CPU::OP_CB_RES_2_D() { res(2, D); } void CPU::OP_CB_RES_2_E() { res(2, E); }
void CPU::OP_CB_RES_2_H() { res(2, H); } void CPU::OP_CB_RES_2_L() { res(2, L); }
void CPU::OP_CB_RES_2_aHL() { Byte val = read(getHL()); res(2, val); write(getHL(), val); } void CPU::OP_CB_RES_2_A() { res(2, A); }

void CPU::OP_CB_RES_3_B() { res(3, B); } void CPU::OP_CB_RES_3_C() { res(3, C); }
void CPU::OP_CB_RES_3_D() { res(3, D); } void CPU::OP_CB_RES_3_E() { res(3, E); }
void CPU::OP_CB_RES_3_H() { res(3, H); } void CPU::OP_CB_RES_3_L() { res(3, L); }
void CPU::OP_CB_RES_3_aHL() { Byte val = read(getHL()); res(3, val); write(getHL(), val); } void CPU::OP_CB_RES_3_A() { res(3, A); }

// =========================================================
// Opcodes CB: 0xA0 - 0xAF (Bits 4 y 5)
// =========================================================
void CPU::OP_CB_RES_4_B() { res(4, B); } void CPU::OP_CB_RES_4_C() { res(4, C); }
void CPU::OP_CB_RES_4_D() { res(4, D); } void CPU::OP_CB_RES_4_E() { res(4, E); }
void CPU::OP_CB_RES_4_H() { res(4, H); } void CPU::OP_CB_RES_4_L() { res(4, L); }
void CPU::OP_CB_RES_4_aHL() { Byte val = read(getHL()); res(4, val); write(getHL(), val); } void CPU::OP_CB_RES_4_A() { res(4, A); }

void CPU::OP_CB_RES_5_B() { res(5, B); } void CPU::OP_CB_RES_5_C() { res(5, C); }
void CPU::OP_CB_RES_5_D() { res(5, D); } void CPU::OP_CB_RES_5_E() { res(5, E); }
void CPU::OP_CB_RES_5_H() { res(5, H); } void CPU::OP_CB_RES_5_L() { res(5, L); }
void CPU::OP_CB_RES_5_aHL() { Byte val = read(getHL()); res(5, val); write(getHL(), val); } void CPU::OP_CB_RES_5_A() { res(5, A); }

// =========================================================
// Opcodes CB: 0xB0 - 0xBF (Bits 6 y 7)
// =========================================================
void CPU::OP_CB_RES_6_B() { res(6, B); } void CPU::OP_CB_RES_6_C() { res(6, C); }
void CPU::OP_CB_RES_6_D() { res(6, D); } void CPU::OP_CB_RES_6_E() { res(6, E); }
void CPU::OP_CB_RES_6_H() { res(6, H); } void CPU::OP_CB_RES_6_L() { res(6, L); }
void CPU::OP_CB_RES_6_aHL() { Byte val = read(getHL()); res(6, val); write(getHL(), val); } void CPU::OP_CB_RES_6_A() { res(6, A); }

void CPU::OP_CB_RES_7_B() { res(7, B); } void CPU::OP_CB_RES_7_C() { res(7, C); }
void CPU::OP_CB_RES_7_D() { res(7, D); } void CPU::OP_CB_RES_7_E() { res(7, E); }
void CPU::OP_CB_RES_7_H() { res(7, H); } void CPU::OP_CB_RES_7_L() { res(7, L); }
void CPU::OP_CB_RES_7_aHL() { Byte val = read(getHL()); res(7, val); write(getHL(), val); } void CPU::OP_CB_RES_7_A() { res(7, A); }


// =========================================================
// Opcodes CB: 0xC0 - 0xCF (Bits 0 y 1)
// =========================================================
void CPU::OP_CB_SET_0_B() { set(0, B); } void CPU::OP_CB_SET_0_C() { set(0, C); }
void CPU::OP_CB_SET_0_D() { set(0, D); } void CPU::OP_CB_SET_0_E() { set(0, E); }
void CPU::OP_CB_SET_0_H() { set(0, H); } void CPU::OP_CB_SET_0_L() { set(0, L); }
void CPU::OP_CB_SET_0_aHL() { Byte val = read(getHL()); set(0, val); write(getHL(), val); } void CPU::OP_CB_SET_0_A() { set(0, A); }

void CPU::OP_CB_SET_1_B() { set(1, B); } void CPU::OP_CB_SET_1_C() { set(1, C); }
void CPU::OP_CB_SET_1_D() { set(1, D); } void CPU::OP_CB_SET_1_E() { set(1, E); }
void CPU::OP_CB_SET_1_H() { set(1, H); } void CPU::OP_CB_SET_1_L() { set(1, L); }
void CPU::OP_CB_SET_1_aHL() { Byte val = read(getHL()); set(1, val); write(getHL(), val); } void CPU::OP_CB_SET_1_A() { set(1, A); }

void CPU::OP_CB_SET_2_B() { set(2, B); } void CPU::OP_CB_SET_2_C() { set(2, C); }
void CPU::OP_CB_SET_2_D() { set(2, D); } void CPU::OP_CB_SET_2_E() { set(2, E); }
void CPU::OP_CB_SET_2_H() { set(2, H); } void CPU::OP_CB_SET_2_L() { set(2, L); }
void CPU::OP_CB_SET_2_aHL() { Byte val = read(getHL()); set(2, val); write(getHL(), val); } void CPU::OP_CB_SET_2_A() { set(2, A); }

void CPU::OP_CB_SET_3_B() { set(3, B); } void CPU::OP_CB_SET_3_C() { set(3, C); }
void CPU::OP_CB_SET_3_D() { set(3, D); } void CPU::OP_CB_SET_3_E() { set(3, E); }
void CPU::OP_CB_SET_3_H() { set(3, H); } void CPU::OP_CB_SET_3_L() { set(3, L); }
void CPU::OP_CB_SET_3_aHL() { Byte val = read(getHL()); set(3, val); write(getHL(), val); } void CPU::OP_CB_SET_3_A() { set(3, A); }

void CPU::OP_CB_SET_4_B() { set(4, B); } void CPU::OP_CB_SET_4_C() { set(4, C); }
void CPU::OP_CB_SET_4_D() { set(4, D); } void CPU::OP_CB_SET_4_E() { set(4, E); }
void CPU::OP_CB_SET_4_H() { set(4, H); } void CPU::OP_CB_SET_4_L() { set(4, L); }
void CPU::OP_CB_SET_4_aHL() { Byte val = read(getHL()); set(4, val); write(getHL(), val); } void CPU::OP_CB_SET_4_A() { set(4, A); }

void CPU::OP_CB_SET_5_B() { set(5, B); } void CPU::OP_CB_SET_5_C() { set(5, C); }
void CPU::OP_CB_SET_5_D() { set(5, D); } void CPU::OP_CB_SET_5_E() { set(5, E); }
void CPU::OP_CB_SET_5_H() { set(5, H); } void CPU::OP_CB_SET_5_L() { set(5, L); }
void CPU::OP_CB_SET_5_aHL() { Byte val = read(getHL()); set(5, val); write(getHL(), val); } void CPU::OP_CB_SET_5_A() { set(5, A); }

void CPU::OP_CB_SET_6_B() { set(6, B); } void CPU::OP_CB_SET_6_C() { set(6, C); }
void CPU::OP_CB_SET_6_D() { set(6, D); } void CPU::OP_CB_SET_6_E() { set(6, E); }
void CPU::OP_CB_SET_6_H() { set(6, H); } void CPU::OP_CB_SET_6_L() { set(6, L); }
void CPU::OP_CB_SET_6_aHL() { Byte val = read(getHL()); set(6, val); write(getHL(), val); } void CPU::OP_CB_SET_6_A() { set(6, A); }

void CPU::OP_CB_SET_7_B() { set(7, B); } void CPU::OP_CB_SET_7_C() { set(7, C); }
void CPU::OP_CB_SET_7_D() { set(7, D); } void CPU::OP_CB_SET_7_E() { set(7, E); }
void CPU::OP_CB_SET_7_H() { set(7, H); } void CPU::OP_CB_SET_7_L() { set(7, L); }
void CPU::OP_CB_SET_7_aHL() { Byte val = read(getHL()); set(7, val); write(getHL(), val); } void CPU::OP_CB_SET_7_A() { set(7, A); }