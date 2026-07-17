#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include "../src/bus.h"
#include "../src/cpu.h"

namespace fs = std::filesystem;

int main () {
    std::string blarggDirectory = "roms/Blargg/";

    std::vector<std::string> filenames;

    for (const auto& entry: fs::directory_iterator(blarggDirectory)) {
        std::string romName = entry.path().filename().string();
        filenames.emplace_back(romName);
    }


    for (const std::string& rom: filenames) {
        Bus gbBus;
        if (!gbBus.cartridge.loadROM(blarggDirectory + rom)) {
            std::cout << "No se encontró " << rom;
            continue;
        }
        
        CPU cpu(gbBus);

        cpu.reset();

        bool testFinished = false;
        long long maxCycles = 2000000000;

        std::cout << "==== TEST " << rom << " ====\n";

        try {
            while (!testFinished && cpu.getCycles() < maxCycles) {
                long long cyclesBefore = cpu.getCycles();
                
                cpu.step();
                
                long long cyclesAfter = cpu.getCycles();
                long long deltaCycles = cyclesAfter - cyclesBefore;

                gbBus.updateTimers(deltaCycles);
                gbBus.ppu.step(deltaCycles);
                cpu.handleInterrupts();

                std::string output = gbBus.getSerialOutput();
                if (output.find("Passed") != std::string::npos) {
                    std::cout << "\n[RESULTADO]: ✅ PASSED\n";
                    testFinished = true;
                } 
                else if (output.find("Failed") != std::string::npos) {
                    std::cout << "\n[RESULTADO]: ❌ FAILED\n";
                    std::cout << "--- REPORTE DE BLARGG ---\n";
                    std::cout << output << "\n";
                    std::cout << "-------------------------\n";
                    testFinished = true;
                }
            }

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