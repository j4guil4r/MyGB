#include "apu.h"
#include <iostream>

static SDL_AudioDeviceID audioDevice = 0;

APU::APU(){
    soundEnabled = false;

    // --- CONFIGURACIÓN DE SDL AUDIO ---
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec); // Limpia la estructura
    desiredSpec.freq = 44100;          // Frecuencia de muestreo estándar (CD)
    desiredSpec.format = AUDIO_F32SYS; // Audio de 32-bits en punto flotante
    desiredSpec.channels = 2;          // 2 canales (Estéreo)
    desiredSpec.samples = 1024;        // Tamaño del buffer interno de SDL
    desiredSpec.callback = nullptr;    // Usaremos SDL_QueueAudio (sin callback complejo)

    SDL_AudioSpec obtainedSpec;
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);

    if (audioDevice == 0){
        std::cerr << "Error al inicializar SDL Audio: " << SDL_GetError() << "\n";
    }
    else {
        SDL_PauseAudioDevice(audioDevice, 0);
        std::cout << "APU Inicializada. Audio Device ID: " << audioDevice << "\n";
    }

}

APU::~APU() {
    if (audioDevice != 0) {
        SDL_CloseAudioDevice(audioDevice);
    }
}

Byte APU::read(Word address) const {
    // Cuando leemos un hardware desconectado o no implementado, la Game Boy devuelve 0xFF
    return 0xFF;
}

void APU::write(Word address, Byte value) {
    // NR52 (0xFF26) - El interruptor maestro de sonido
    if (address == 0xFF26) {
        // Si el bit 7 está en 1, el audio se enciende. Si está en 0, se apaga.
        soundEnabled = ((value >> 7) & 1) == 1;
    }
}

void APU::step(int cycles) {
    if (!soundEnabled) return;

    // TODO: Usar 'cycles' para decrementar los temporizadores de los 4 canales.
    // TODO: Sintetizar la onda final (mezclar canales).
    // TODO: Insertar las muestras en 'audioBuffer'.
    
    // Si tenemos suficientes muestras en nuestro buffer local, las enviamos a SDL
    if (audioBuffer.size() >= 1024) {
        SDL_QueueAudio(audioDevice, audioBuffer.data(), audioBuffer.size() * sizeof(float));
        audioBuffer.clear();
    }
}