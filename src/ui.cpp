#include "ui.h"

UI::UI(){}

UI::~UI(){
    if (audioDevice != 0) {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool UI::init(int width, int height, int scale) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow(
        "Game Boy Emulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        width * scale, height * scale, 
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        width, height
    );

    if (!window || !renderer || !texture) {
        std::cerr << "Error creando la ventana/renderizador: " << SDL_GetError() << "\n";
        SDL_Quit();
        return false;
    }

    // --- CONFIGURACIÓN DE AUDIO SDL ---
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_F32SYS;
    desiredSpec.channels = 2;
    desiredSpec.samples = 1024;
    desiredSpec.callback = nullptr;

    SDL_AudioSpec obtainedSpec;
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);

    if (audioDevice == 0) {
        std::cerr << "Advertencia: No se pudo abrir dispositivo de audio SDL: " << SDL_GetError() << "\n";
    } else {
        SDL_PauseAudioDevice(audioDevice, 0);
    }

    return true;
}

void UI::handleEvents(Bus& gbBus) {
    SDL_Event event;

    // A. Atender eventos de la ventana
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            bool buttonPressed = false;
            // Al presionar, apagamos el bit (ponemos 0) con un AND bitwise y negación (~)
            switch (event.key.keysym.sym) {
                case SDLK_RIGHT: gbBus.joypadDir &= ~0x01; buttonPressed = true; break;
                case SDLK_LEFT:  gbBus.joypadDir &= ~0x02; buttonPressed = true; break;
                case SDLK_UP:    gbBus.joypadDir &= ~0x04; buttonPressed = true; break;
                case SDLK_DOWN:  gbBus.joypadDir &= ~0x08; buttonPressed = true; break;
                case SDLK_z:     gbBus.joypadAction &= ~0x01; buttonPressed = true; break; // Botón A
                case SDLK_x:     gbBus.joypadAction &= ~0x02; buttonPressed = true; break; // Botón B
                case SDLK_a:     gbBus.joypadAction &= ~0x04; buttonPressed = true; break; // Select (Tecla A)
                case SDLK_s:     gbBus.joypadAction &= ~0x08; buttonPressed = true; break; // Start  (Tecla S)
            }
            
            // Si presionamos un botón, disparamos la Interrupción del Joypad (Bit 4 de IF)
            if (buttonPressed) {
                Byte currentIF = gbBus.read(0xFF0F);
                gbBus.write(0xFF0F, currentIF | 0x10);
            }
        } 
        else if (event.type == SDL_KEYUP) {
            // Al soltar, encendemos el bit (ponemos 1) con un OR bitwise
            switch (event.key.keysym.sym) {
                case SDLK_RIGHT: gbBus.joypadDir |= 0x01; break;
                case SDLK_LEFT:  gbBus.joypadDir |= 0x02; break;
                case SDLK_UP:    gbBus.joypadDir |= 0x04; break;
                case SDLK_DOWN:  gbBus.joypadDir |= 0x08; break;
                case SDLK_z:     gbBus.joypadAction |= 0x01; break; // Botón A
                case SDLK_x:     gbBus.joypadAction |= 0x02; break; // Botón B
                case SDLK_a:     gbBus.joypadAction |= 0x04; break; // Select (Tecla A)
                case SDLK_s:     gbBus.joypadAction |= 0x08; break; // Start  (Tecla S)
            }
        }
    }
}

void UI::render(Bus& gbBus) {
    if (gbBus.ppu.frameReady) {
        SDL_UpdateTexture(texture, nullptr, gbBus.ppu.framebuffer.data(), GB_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        gbBus.ppu.frameReady = false;
    } else {
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(texture, nullptr, gbBus.ppu.framebuffer.data(), GB_WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}

void UI::syncAudio(Bus& gbBus) {
    if (audioDevice == 0) return;

    const Uint32 MAX_AUDIO_QUEUE_BYTES = 16384;
    Uint32 queued = SDL_GetQueuedAudioSize(audioDevice);
    
    float tempBuffer[2048];
    while (gbBus.apu.getAvailableAudioSamples() > 0) {
        size_t available = gbBus.apu.getAvailableAudioSamples();
        size_t toRead = std::min(available, sizeof(tempBuffer) / sizeof(tempBuffer[0]));
        size_t readCount = gbBus.apu.readAudioSamples(tempBuffer, toRead);

        if (readCount > 0 && queued < MAX_AUDIO_QUEUE_BYTES) {
            SDL_QueueAudio(audioDevice, tempBuffer, static_cast<Uint32>(readCount * sizeof(float)));
            queued += static_cast<Uint32>(readCount * sizeof(float));
        }
    }
}