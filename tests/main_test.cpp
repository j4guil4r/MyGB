#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <chrono>
#include <algorithm>
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "=== MyGB Test Runner (Blargg Suites) ===\n";
        std::cout << "Uso: " << argv[0] << " <ruta_a_rom.gb | ruta_a_directorio>\n\n";
        std::cout << "Comportamiento:\n";
        std::cout << "  - Si recibe un archivo .gb: ejecuta únicamente ese test.\n";
        std::cout << "  - Si recibe un directorio: busca recursivamente y ejecuta todas las ROMs .gb dentro.\n\n";
        std::cout << "Ejemplos:\n";
        std::cout << "  " << argv[0] << " build/gb-test-roms/cpu_instrs/individual/01-special.gb\n";
        std::cout << "  " << argv[0] << " build/gb-test-roms/cpu_instrs/individual/\n";
        return 0;
    }

    fs::path targetPath(argv[1]);

    if (!fs::exists(targetPath)) {
        std::cerr << "Error: La ruta especificada no existe: " << targetPath.string() << "\n";
        return 1;
    }

    std::vector<std::string> testFiles;

    if (fs::is_regular_file(targetPath)) {
        if (targetPath.extension() == ".gb") {
            testFiles.push_back(targetPath.string());
        } else {
            std::cerr << "Error: El archivo especificado no tiene extensión .gb: " << targetPath.string() << "\n";
            return 1;
        }
    } else if (fs::is_directory(targetPath)) {
        for (const auto& entry : fs::recursive_directory_iterator(targetPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".gb") {
                testFiles.push_back(entry.path().string());
            }
        }
        std::sort(testFiles.begin(), testFiles.end());
    } else {
        std::cerr << "Error: La ruta no es un archivo ni un directorio válido: " << targetPath.string() << "\n";
        return 1;
    }

    if (testFiles.empty()) {
        std::cout << "No se encontraron archivos .gb en: " << targetPath.string() << "\n";
        return 0;
    }

    std::cout << "Iniciando batería de pruebas (" << testFiles.size() << " test(s) encontrado(s))...\n\n";

    for (const std::string& filepath : testFiles) {
        fs::path pathObj(filepath);
        auto inicio = std::chrono::high_resolution_clock::now();
        
        runBlarggTest(filepath, pathObj.filename().string());
        
        auto fin = std::chrono::high_resolution_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
        std::cout << pathObj.filename().string() << " terminó en " << duracion.count() << " ms\n\n";
    }

    return 0;
}