#include <format>
#include <iostream>
#include <string>
#include <filesystem>
#include <SDL2/SDL.h>
#include "bus.h"
#include "cpu.h"
#include "ppu.h"

namespace fs = std::filesystem;

void Test () {
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
        long long maxCycles = 2000000000;

        std::cout << "==== TEST " << rom << " ====\n";

        try {
            // Loop de ejecución
            while (!testFinished && cpu.getCycles() < maxCycles) {
                long long cyclesBefore = cpu.getCycles();
                
                cpu.step();
                
                long long cyclesAfter = cpu.getCycles();
                long long deltaCycles = cyclesAfter - cyclesBefore;

                gbBus.updateTimers(deltaCycles);
                gbBus.ppu.step(deltaCycles);
                cpu.handleInterrupts();

                // Revisar la salida serial para detener el bucle
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

            // Si el while terminó porque superó el límite de maxCycles:
            if (!testFinished) {
                std::cout << "\n[RESULTADO]: ⏱️ TIMEOUT (Posible bucle infinito o test muy largo)\n";
            }

        } catch (const std::exception& e) {
            // Si OP_UNKNOWN hace throw, el bucle se rompe y cae aquí
            std::cout << "\n[RESULTADO]: 💥 CRASH -> " << e.what() << "\n";
        }
    }
}

int Window () {
    // 1. Inicializar SDL
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

    // 2. Crear el Renderizador (Acelerado por Hardware)
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // 3. Crear la Textura (Nuestro lienzo de 160x144)
    // Usamos SDL_PIXELFORMAT_ARGB8888 porque nuestro framebuffer es un array de uint32_t
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        GB_WIDTH, GB_HEIGHT
    );

    // Instanciamos nuestra PPU
    // TODO: conectar al bus
    PPU ppu;

    // --- TEST VISUAL ---
    // Vamos a pintar un par de píxeles a mano en el framebuffer para probar que funciona.
    // El índice se calcula así: (Y * ancho) + X
    ppu.framebuffer[(72 * GB_WIDTH) + 80] = 0xFFFF0000; // Píxel Rojo exacto en el centro
    ppu.framebuffer[(73 * GB_WIDTH) + 80] = 0xFF00FF00; // Píxel Verde debajo
    ppu.framebuffer[(74 * GB_WIDTH) + 80] = 0xFF0000FF; // Píxel Azul debajo

    bool isRunning = true;
    SDL_Event event;

    // 4. El Game Loop Principal
    while (isRunning) {
        // Atender eventos (como hacer clic en la 'X' para cerrar la ventana)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        // --- RENDERIZADO ---
        // A. Actualizamos la Textura de SDL con los datos de nuestro Framebuffer
        // El "Pitch" es la cantidad de bytes por cada fila (160 píxeles * 4 bytes)
        SDL_UpdateTexture(texture, nullptr, ppu.framebuffer.data(), GB_WIDTH * sizeof(uint32_t));

        // B. Limpiamos el renderizador
        SDL_RenderClear(renderer);

        // C. Copiamos la Textura al Renderizador (SDL la escala automáticamente x4)
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        // D. Presentamos lo dibujado en la pantalla
        SDL_RenderPresent(renderer);

        // E. Pequeña pausa para no quemar el CPU (aprox 60 FPS)
        SDL_Delay(16); 
    }

    // 5. Limpieza al salir
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    window = NULL;
    renderer = NULL; 
    texture = NULL;

    return 0;
}


int main () {
    Test();
    Window();
    return 0;
}
