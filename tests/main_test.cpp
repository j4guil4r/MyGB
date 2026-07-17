#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include "../src/bus.h"
#include "../src/cpu.h"

namespace fs = std::filesystem;

void runBlarggTest(const std::string& romPath, const std::string& romName) {
    Bus gbBus;
    if (!gbBus.cartridge.loadROM(romPath)) {
        std::cout << "❌ No se encontró " << romName << "\n";
        return;
    }
    
    CPU cpu(gbBus);
    cpu.reset();

    bool testFinished = false;
    long long maxCycles = 150000000;

    std::cout << "==== TEST " << romName << " ====\n";

    try {
        while (!testFinished && cpu.getCycles() < maxCycles) {
            long long cyclesBefore = cpu.getCycles();
            
            cpu.step();
            
            long long deltaCycles = cpu.getCycles() - cyclesBefore;

            gbBus.updateTimers(deltaCycles);
            gbBus.ppu.step(deltaCycles);
            gbBus.apu.step(deltaCycles);

            if (gbBus.ppu.requestVBlankInterrupt) {
                Byte currentIF = gbBus.read(0xFF0F);
                gbBus.write(0xFF0F, currentIF | 0x01); 
                gbBus.ppu.requestVBlankInterrupt = false; 
            }

            if (gbBus.ppu.requestStatInterrupt) {
                gbBus.write(0xFF0F, gbBus.read(0xFF0F) | 0x02); 
                gbBus.ppu.requestStatInterrupt = false;
            }
            
            cpu.handleInterrupts();
            
            std::string output = gbBus.getSerialOutput();
            if (output.find("Passed") != std::string::npos) {
                std::cout << "[RESULTADO]: ✅ PASSED\n\n";
                testFinished = true;
            } 
            else if (output.find("Failed") != std::string::npos) {
                std::cout << "[RESULTADO]: ❌ FAILED\n";
                std::cout << "--- REPORTE ---\n" << output << "\n---------------\n\n";
                testFinished = true;
            }
        }

        if (!testFinished) {
            std::cout << "[RESULTADO]: ⏱️ TIMEOUT\n";
            std::cout << "--- CPU CONGELADA EN ---\n";
            std::cout << "PC Actual: 0x" << std::hex << cpu.getPC() << std::dec << "\n";
            std::cout << "LY Actual (0xFF44): " << (int)gbBus.read(0xFF44) << "\n";
            std::cout << "------------------------------------------------\n\n";
        }

    } catch (const std::exception& e) {
        std::cout << "[RESULTADO]: 💥 CRASH -> " << e.what() << "\n\n";
    }
}

int main () {
    std::string baseDirectory = "/home/joseag/Projects/MyGB/build/gb-test-roms/dmg_sound";
    std::vector<std::string> testFiles;

    for (const auto& entry : fs::recursive_directory_iterator(baseDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gb") {
            std::string parentDir = entry.path().parent_path().filename().string();
            if (parentDir == "individual" || parentDir == "rom_singles") {
                testFiles.push_back(entry.path().string());
            }
        }
    }

    std::cout << "Iniciando batería de pruebas Blargg...\n";
    std::cout << "Se encontraron " << testFiles.size() << " tests individuales.\n\n";

    for (const std::string& filepath : testFiles) {
        fs::path pathObj(filepath); 
        runBlarggTest(filepath, pathObj.filename().string());
    }

    return 0;
}