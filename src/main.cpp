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

        int cyclesThisFrame = 0;
        
        while (cyclesThisFrame < MAX_CYCLES_PER_FRAME) {
            long long cyclesBefore = cpu.getCycles();
            
            cpu.step();
            cpu.handleInterrupts();
            
            cyclesThisFrame += cpu.getCycles() - cyclesBefore;
        }

        if (gbBus.ppu.frameReady) {
            ui.render(gbBus);
            gbBus.ppu.frameReady = false; 
        }

        ui.syncAudio(gbBus);

        int frameTime = static_cast<int>(SDL_GetTicks() - frameStart);

        if (frameTime < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frameTime);
        }
    }

    std::cout << "Emulador cerrado correctamente.\n";
    return 0;
}