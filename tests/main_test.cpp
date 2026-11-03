#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <chrono>
#include "../src/bus.h"
#include "../src/cpu.h"

namespace fs = std::filesystem;

void printVRAMDump(Bus& gbBus) {
    std::cout << "\n=== PANTALLA DE LA GAME BOY (VRAM) ===\n";
    bool bgMap = (gbBus.read(0xFF40) & 0x08) != 0;
    Word mapAddr = bgMap ? 0x9C00 : 0x9800;
    
    for(int i = 0; i < 32 * 32; i++) {
        char tile = gbBus.read(mapAddr + i);
        if (tile >= 32 && tile <= 126) std::cout << tile;
        else std::cout << " ";
        
        if ((i + 1) % 32 == 0) std::cout << "\n";
    }
    std::cout << "======================================\n";
}

bool checkInfiniteLoop(Bus& gbBus, Word currentPC) {
    Byte op = gbBus.read(currentPC);
    Byte arg = gbBus.read(currentPC + 1);

    if (op == 0x18 && arg == 0xFE) { // JR -2
        std::cout << "\n==================================\n";
        std::cout << "✅ TEST FINALIZADO EN PC: 0x" << std::hex << currentPC << std::dec << "\n";
        printVRAMDump(gbBus);
        return true;
    }
    return false;
}

bool checkSerialOutput(Bus& gbBus) {
    std::string output = gbBus.getSerialOutput();
    if (output.find("Passed") != std::string::npos) {
        std::cout << "[RESULTADO]: ✅ PASSED\n\n";
        return true;
    } 
    else if (output.find("Failed") != std::string::npos) {
        std::cout << "[RESULTADO]: ❌ FAILED\n";
        std::cout << "--- REPORTE ---\n" << output << "\n---------------\n\n";
        return true;
    }
    return false;
}

void handleTimeout(CPU& cpu, Bus& gbBus) {
    std::cout << "[RESULTADO]: ⏱️ TIMEOUT\n";
    std::cout << "--- LOG INTERCEPTADO ANTES DEL CONGELAMIENTO ---\n";
    std::cout << gbBus.getSerialOutput() << "\n";
    std::cout << "--- CPU CONGELADA EN ---\n";
    std::cout << "PC Actual: 0x" << std::hex << cpu.getPC() << std::dec << "\n";
    std::cout << "LY Actual (0xFF44): " << (int)gbBus.read(0xFF44) << "\n";
    std::cout << "------------------------------------------------\n";
    printVRAMDump(gbBus);
}

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
            Word currentPC = cpu.getPC();

            if (checkInfiniteLoop(gbBus, currentPC)) break;

            cpu.step();
            cpu.handleInterrupts();

            if (checkSerialOutput(gbBus)) break;
        }

        if (cpu.getCycles() >= maxCycles) {
            handleTimeout(cpu, gbBus);
        }

    } catch (const std::exception& e) {
        std::cout << "[RESULTADO]: 💥 CRASH -> " << e.what() << "\n\n";
    }
}

int main () {
    std::string baseDirectory = "/home/joseag/Projects/MyGB/build/gb-test-roms";
    std::vector<std::string> testFiles;

    for (const auto& entry : fs::recursive_directory_iterator(baseDirectory)) {
        if (entry.path().string().find("oam_bug") != std::string::npos) continue;
        if (entry.is_regular_file() && entry.path().extension() == ".gb") {
            std::string parentDir = entry.path().parent_path().filename().string();
            if (parentDir == "individual" || parentDir == "rom_singles") {
                //if(entry.path().filename() == "7-timing_effect.gb") 
                    testFiles.push_back(entry.path().string());
            }
        }
    }

    std::cout << "Iniciando batería de pruebas Blargg...\n";
    std::cout << "Se encontraron " << testFiles.size() << " tests individuales.\n\n";

    for (const std::string& filepath : testFiles) {
        fs::path pathObj(filepath);
        auto inicio = std::chrono::high_resolution_clock::now();
        
        runBlarggTest(filepath, pathObj.filename().string());
        
        auto fin = std::chrono::high_resolution_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
        std::cout << pathObj.filename().string() << " terminó en " << duracion.count() << " ms\n";
    }

    return 0;
}