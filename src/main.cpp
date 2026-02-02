#include <format>
#include <iostream>
#include "bus.h"
#include "cpu.h"

int main () {
    // 1. Hardware Setup
    Bus gbBus;
    CPU cpu(gbBus);

    // 2. Reseteamos la CPU (PC = 0x0100)
    cpu.reset(); // PC vuelve a 0x0100

    // Instrucción 1: NOP (0x00)
    gbBus.write(0x0100, 0x00);

    // Instrucción 2: JP 0x0100
    gbBus.write(0x0101, 0xC3); // Opcode JP
    gbBus.write(0x0102, 0x00); // Low byte de la dirección (00)
    gbBus.write(0x0103, 0x01); // High byte de la dirección (01) -> 0x0100

    std::cout << "--- TEST JUMP (Infinite Loop) ---\n";

    // Ejecutamos varios pasos para ver como el PC va y vuelve
    for (int i = 0; i < 6; i++) {
        std::cout << std::format("Step {}: PC:{:04X}\n", i, cpu.PC);
        cpu.step();
    }
    return 0;
}