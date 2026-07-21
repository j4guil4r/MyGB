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
        // --- INICIO DE LA MÁQUINA DEL TIEMPO ---
        //Word pcHistory[30] = {0};
        //int historyIdx = 0;
        while (!testFinished && cpu.getCycles() < maxCycles) {
            long long cyclesBefore = cpu.getCycles();

            Word currentPC = cpu.getPC();

            // --- DETECTOR UNIVERSAL DE FIN DE TEST ---
            // Leemos la instrucción actual para ver si es un JR -2 (0x18 0xFE)
            Byte op = gbBus.read(currentPC);
            Byte arg = gbBus.read(currentPC + 1);

            if (op == 0x18 && arg == 0xFE) {
                // ¡La CPU se ha encerrado a sí misma a propósito!
                Byte testResult = gbBus.read(0xA000);
                
                std::cout << "\n==================================\n";
                std::cout << "TEST FINALIZADO EN PC: 0x" << std::hex << currentPC << std::dec << "\n";
                
                if (testResult == 0x80) {
                    std::cout << "⚠️ BUCLE ALCANZADO, PERO EL TEST SIGUE EN CURSO (0x80).\n";
                } else if (testResult == 0x00) {
                    std::cout << "✅ [RESULTADO]: ¡TEST SUPERADO (PASSED)!\n";
                    testFinished = true;
                } else {
                    std::cout << "❌ [RESULTADO]: FAILED (Error Código: " << (int)testResult << ")\n";
                    std::cout << "MENSAJE DE BLARGG: ";
                    Word textAddress = 0xA004;
                    while (true) {
                        char c = (char)gbBus.read(textAddress++);
                        if (c == '\0' || textAddress > 0xBFFF) break;
                        std::cout << c;
                    }
                    std::cout << "\n";
                    testFinished = true;
                }
                std::cout << "==================================\n";
                
                break; // Rompemos el bucle de tu emulador limpiamente
            }
            // Guardamos el PC actual en el historial circular
            //pcHistory[historyIdx] = currentPC;
            //historyIdx = (historyIdx + 1) % 30;
            
            cpu.step();
            
            long long deltaCycles = cpu.getCycles() - cyclesBefore;

            gbBus.updateTimers(deltaCycles);
            gbBus.ppu.step(deltaCycles);
            //gbBus.apu.step(deltaCycles);
            gbBus.apu.syncTo(gbBus.systemCycles);

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
            std::cout << "--- LOG INTERCEPTADO ANTES DEL CONGELAMIENTO ---\n";
            std::cout << gbBus.getSerialOutput() << "\n";
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
    std::string baseDirectory = "/home/joseag/Projects/MyGB/build/gb-test-roms/dmg_sound/rom_singles";
    std::vector<std::string> testFiles;

    for (const auto& entry : fs::recursive_directory_iterator(baseDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gb") {
            //std::string parentDir = entry.path().parent_path().filename().string();
            //if (parentDir == "individual" || parentDir == "rom_singles") {
                //if(entry.path().filename() == "09-wave read while on.gb") 
                    testFiles.push_back(entry.path().string());
            //}
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