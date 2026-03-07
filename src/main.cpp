#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include "bus.h"
#include "cpu.h"

int main(int argc, char* argv[]) {
    // 1. INICIALIZAR SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << "\n";
        return -1;
    }

    int scale = 4;
    SDL_Window* window = SDL_CreateWindow(
        "Game Boy Emulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        GB_WIDTH * scale, GB_HEIGHT * scale, 
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        GB_WIDTH, GB_HEIGHT
    );

    if (!window || !renderer || !texture) {
        std::cerr << "Error creando la ventana/renderizador: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    Bus gbBus;
    std::string romPath = "roms/Tetris.gb"; 
    
    if (!gbBus.loadROM(romPath)) {
        std::cerr << "No se pudo cargar la ROM: " << romPath << "\n";
        return -1;
    }

    CPU cpu(gbBus);
    cpu.reset();

    // Configuración post-BIOS
    cpu.A = 0x01; cpu.F = 0xB0;
    cpu.B = 0x00; cpu.C = 0x13;
    cpu.D = 0x00; cpu.E = 0xD8;
    cpu.H = 0x01; cpu.L = 0x4D;
    cpu.SP = 0xFFFE;
    cpu.PC = 0x0100;

    // 3. EL BUCLE PRINCIPAL (GAME LOOP)
    bool isRunning = true;
    SDL_Event event;

    std::cout << "Emulador iniciado. Jugando: " << romPath << "\n";

    // 70224 ciclos de reloj de la CPU equivalen exactamente a 1 frame (1/60 de segundo)
    const int MAX_CYCLES_PER_FRAME = 70224; 
    int framesRenderizados = 0;

    while (isRunning) {
        // A. Atender eventos de la ventana
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        // B. Ejecutar la CPU por exactamente 1 frame de tiempo
        int cyclesThisFrame = 0;
        
        while (cyclesThisFrame < MAX_CYCLES_PER_FRAME) {
            long long cyclesBefore = cpu.getCycles();
            
            cpu.step(); // Ejecutamos 1 instrucción
            
            long long deltaCycles = cpu.getCycles() - cyclesBefore;
            cyclesThisFrame += deltaCycles;

            gbBus.updateTimers(deltaCycles);
            gbBus.ppu.step(deltaCycles); 

            // Procesar interrupciones de la PPU
            if (gbBus.ppu.requestVBlankInterrupt) {
                Byte currentIF = gbBus.read(0xFF0F);
                gbBus.write(0xFF0F, currentIF | 0x01); 
                gbBus.ppu.requestVBlankInterrupt = false; 
            }

            cpu.handleInterrupts();
        }

        // C. Terminó el frame. ¿La PPU armó un cuadro nuevo?
        if (gbBus.ppu.frameReady) {
            SDL_UpdateTexture(texture, nullptr, gbBus.ppu.framebuffer.data(), GB_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);

            gbBus.ppu.frameReady = false;
        } else {
            // Si la pantalla estaba apagada, igual refrescamos la ventana para que no se congele el OS
            SDL_RenderClear(renderer);
            SDL_UpdateTexture(texture, nullptr, gbBus.ppu.framebuffer.data(), GB_WIDTH * sizeof(uint32_t));
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }

        // D. RADAR DE DEPURACIÓN (Imprimir el PC cada 60 frames / 1 segundo)
        framesRenderizados++;
        if (framesRenderizados % 60 == 0) {
            std::cout << "[RADAR] El juego sigue corriendo. PC actual: 0x" 
                      << std::hex << cpu.PC << std::dec << "\n";
        }

        // E. Sincronizar a 60 FPS
        SDL_Delay(16); 
    }


    // 4. LIMPIEZA
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Emulador cerrado correctamente.\n";
    return 0;
}