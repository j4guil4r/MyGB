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

    if (!gbBus.loadROM("01-special.gb")) {
        return -1; // Salir si falla
    }

    std::cout << "--- INICIO DEL TEST BLARGG ---\n";

    // Loop de ejecución
    while (true) {
        // Logueamos antes de ejecutar para ver qué va a hacer
        // Nota: Si imprimes en cada ciclo, la consola será LENTÍSIMA.
        // Usa getchar() para ir paso a paso si quieres ver detalle.

        // std::cout << std::format("PC:{:04X} OP:{:02X}\n", cpu.PC, gbBus.read(cpu.PC));

        cpu.step();
        //if (!(cpu.PC % 10))
        //    std::cout << std::hex << cpu.PC << std::endl;

    }
    return 0;
}