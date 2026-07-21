#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include "bus.h"
#include "cpu.h"
#include "ui.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Error: Falta el archivo de la ROM.\n";
        std::cerr << "Uso correcto: " << argv[0] << " <ruta_a_tu_juego.gb>\n";
        std::cerr << "Ejemplo: " << argv[0] << " roms/PokemonRedVersion.gb\n";
        return -1;
    }
    
    std::string romPath = argv[1];
     
    UI ui;
    if (!ui.init(GB_WIDTH, GB_HEIGHT, SCALE)) return -1;

    Bus gbBus;
    
    if (!gbBus.cartridge.loadROM(romPath)) {
        std::cerr << "No se pudo cargar la ROM: " << romPath << "\n";
        return -1;
    }

    CPU cpu(gbBus);
    cpu.reset();

    std::cout << "Emulador iniciado. Jugando: " << romPath << "\n";

    // 70224 ciclos de reloj de la CPU equivalen exactamente a 1 frame (1/60 de segundo)
    const int MAX_CYCLES_PER_FRAME = 70224;
    const int TARGET_FRAME_TIME = 16;

    while (ui.isRunning()) {
        Uint32 frameStart = SDL_GetTicks();

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
            //gbBus.apu.step(deltaCycles); 
            gbBus.apu.syncTo(gbBus.systemCycles);

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

        if (gbBus.ppu.frameReady) {
            ui.render(gbBus);
            gbBus.ppu.frameReady = false; 
        }
        // Terminó el frame. ¿La PPU armó un cuadro nuevo?
        //ui.render(gbBus);
        //gbBus.ppu.frameReady = false;

        int frameTime = static_cast<int>(SDL_GetTicks() - frameStart);

        if (frameTime < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frameTime);
        }
    }

    std::cout << "Emulador cerrado correctamente.\n";
    return 0;
}