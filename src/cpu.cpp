#include <cstdio>

#include "CPU.h"

CPU::CPU(Bus& busReference) : bus(busReference) {
}

void CPU::step() {
    // 1. Fetch
    Byte opcode = fetchByte();

    // 2. Decode & Execute
    switch (opcode) {
        // NOP
        case 0x00:
            cycles += 4;
            break;
        // TODO: Los otros casos.
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