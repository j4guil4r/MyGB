#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include "bus.h"
#include "cpu.h"
#include "ui.h"

int main() {
    UI ui;
    if (!ui.init(GB_WIDTH, GB_HEIGHT, SCALE)) return -1;

    Bus gbBus;
    std::string romPath = "roms/Tetris.gb"; 
    
    if (!gbBus.loadROM(romPath)) {
        std::cerr << "No se pudo cargar la ROM: " << romPath << "\n";
        return -1;
    }

    CPU cpu(gbBus);
    cpu.reset();

    std::cout << "Emulador iniciado. Jugando: " << romPath << "\n";

    // 70224 ciclos de reloj de la CPU equivalen exactamente a 1 frame (1/60 de segundo)
    const int MAX_CYCLES_PER_FRAME = 70224;

    while (ui.isRunning()) {

        ui.handleEvents(gbBus);

        // Ejecutar la CPU por exactamente 1 frame de tiempo
        int cyclesThisFrame = 0;
        
        while (cyclesThisFrame < MAX_CYCLES_PER_FRAME) {
            long long cyclesBefore = cpu.getCycles();
            
            cpu.step(); // Ejecutamos 1 instrucción
            
            long long deltaCycles = cpu.getCycles() - cyclesBefore;
            cyclesThisFrame += deltaCycles;

            gbBus.updateTimers(deltaCycles);
            gbBus.ppu.step(deltaCycles); 

            // Procesar interrupción de V-Blank (Bit 0)
            if (gbBus.ppu.requestVBlankInterrupt) {
                Byte currentIF = gbBus.read(0xFF0F);
                gbBus.write(0xFF0F, currentIF | 0x01); 
                gbBus.ppu.requestVBlankInterrupt = false; 
            }

            // --- Procesar interrupción de STAT (Bit 1) ---
            if (gbBus.ppu.requestStatInterrupt) {
                gbBus.write(0xFF0F, gbBus.read(0xFF0F) | 0x02); 
                gbBus.ppu.requestStatInterrupt = false;
            }

            cpu.handleInterrupts();
        }

        // Terminó el frame. ¿La PPU armó un cuadro nuevo?
        ui.render(gbBus);
        gbBus.ppu.frameReady = false;

        SDL_Delay(16); 
    }

    std::cout << "Emulador cerrado correctamente.\n";
    return 0;
}