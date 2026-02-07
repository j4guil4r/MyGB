#include <format>
#include <iostream>
#include "bus.h"
#include "cpu.h"

int main () {

    Bus gbBus;
    CPU cpu(gbBus);

    cpu.reset();


    // Configuración inicial de registers post-BIOS:
    cpu.A = 0x01;
    cpu.F = 0xB0; // Z=1, N=0, H=1, C=0
    cpu.B = 0x00; cpu.C = 0x13;
    cpu.D = 0x00; cpu.E = 0xD8;
    cpu.H = 0x01; cpu.L = 0x4D;
    cpu.SP = 0xFFFE;
    cpu.PC = 0x0100; // Inicio del juego

    if (!gbBus.loadROM("02-interrupts.gb")) {
        return -1; // Salir si falla
    }

    std::cout << "--- INICIO DEL TEST BLARGG ---\n";

    // Loop de ejecución
    while (true) {
        int ciclosBefore = cpu.getCycles();
        cpu.step();
        int cyclesAfter = cpu.getCycles();
        int deltaCycles = cyclesAfter - ciclosBefore;

        gbBus.updateTimers(deltaCycles);
        cpu.handleInterrupts();
    }
    return 0;
}