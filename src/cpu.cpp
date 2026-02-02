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