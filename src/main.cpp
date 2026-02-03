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

    if (!gbBus.loadROM("dmg_boot.bin")) {
        return -1; // Salir si falla
    }

    std::cout << "--- INICIO DEL BOOT ROM ---\n";

    // Imprimimos los primeros bytes para verificar que cargó
    std::cout << "Primeros bytes en memoria:\n";
    for(int i=0; i<5; i++) {
        std::cout << std::format("{:02X} ", gbBus.read(i));
    }
    std::cout << "\n\n";

    // Loop de ejecución
    while (true) {
        // Logueamos antes de ejecutar para ver qué va a hacer
        // Nota: Si imprimes en cada ciclo, la consola será LENTÍSIMA.
        // Usa getchar() para ir paso a paso si quieres ver detalle.

        // std::cout << std::format("PC:{:04X} OP:{:02X}\n", cpu.PC, gbBus.read(cpu.PC));

        cpu.step();
        std::cout << std::hex << cpu.PC << std::endl;

        // Freno de emergencia para que no sature tu CPU real
        // (temporal, luego controlaremos timing real)
    }
    return 0;
}