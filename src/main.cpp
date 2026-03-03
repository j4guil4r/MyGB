#include <format>
#include <iostream>
#include <string>
#include <filesystem>
#include "bus.h"
#include "cpu.h"

namespace fs = std::filesystem;

int main () {
    std::string romsDirectory = "roms/";

    std::vector<std::string> filenames;

    for (const auto& entry: fs::directory_iterator(romsDirectory)) {
        if(entry.path().extension() == ".gb") {
            std::string romName = entry.path().filename().string();
            filenames.emplace_back(romName);
        }
    }


    for (const std::string& rom: filenames) {

        Bus gbBus;
        if (!gbBus.loadROM(romsDirectory + rom)) {
            std::cout << "No se encontró " << rom;
            continue;
        }
        
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

        bool testFinished = false;
        long long maxCycles = 50000000;

        std::cout << "==== TEST " << rom << " ====\n";

        try {
            // Loop de ejecución
            while (!testFinished && cpu.getCycles() < maxCycles) {
                long long cyclesBefore = cpu.getCycles();
                
                cpu.step();
                
                long long cyclesAfter = cpu.getCycles();
                long long deltaCycles = cyclesAfter - cyclesBefore;

                gbBus.updateTimers(deltaCycles);
                cpu.handleInterrupts();

                // Revisar la salida serial para detener el bucle
                std::string output = gbBus.getSerialOutput();
                if (output.find("Passed") != std::string::npos) {
                    std::cout << "\n[RESULTADO]: ✅ PASSED\n";
                    testFinished = true;
                } 
                else if (output.find("Failed") != std::string::npos) {
                    std::cout << "\n[RESULTADO]: ❌ FAILED\n";
                    testFinished = true;
                }
            }

            // Si el while terminó porque superó el límite de maxCycles:
            if (!testFinished) {
                std::cout << "\n[RESULTADO]: ⏱️ TIMEOUT (Posible bucle infinito o test muy largo)\n";
            }

        } catch (const std::exception& e) {
            // Si OP_UNKNOWN hace throw, el bucle se rompe y cae aquí
            std::cout << "\n[RESULTADO]: 💥 CRASH -> " << e.what() << "\n";
        }
    }
    return 0;
}