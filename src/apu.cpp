#include "apu.h"
#include <iostream>

static SDL_AudioDeviceID audioDevice = 0;

APU::APU(){
    soundEnabled = false;
    sampleCounter = 0;

    // --- INICIALIZAR CANAL 1 ---
    NR10 = 0; NR11 = 0; NR12 = 0; NR13 = 0; NR14 = 0;
    timer1 = 0;
    dutyPointer1 = 0;
    frequency1 = 0;
    volume1 = 0.0f;
    channel1On = false;
    currentVolume1 = 0;
    envelopeTimer1 = 0;
    envelopePeriod1 = 0;
    envelopeDirection1 = 0;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Error crítico: No se pudo inicializar el subsistema de audio SDL: " << SDL_GetError() << "\n";
    }

    // --- CONFIGURACIÓN DE SDL AUDIO ---
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_F32SYS;
    desiredSpec.channels = 2;
    desiredSpec.samples = 1024;
    desiredSpec.callback = nullptr;

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
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

Byte APU::read(Word address) const {
    switch (address) {
        case 0xFF10: return NR10 | 0x80; 
        case 0xFF11: return NR11 | 0x3F; // Solo los 2 bits más altos (Duty) se pueden leer
        case 0xFF12: return NR12;
        case 0xFF13: return 0xFF;        // NR13 es "Write-Only" (Solo escritura)
        case 0xFF14: return NR14 | 0xBF; // Solo el bit 6 (Length enable) se puede leer
        case 0xFF26: return soundEnabled ? 0xFF : 0x7F; 
        default: return 0xFF;
    }
}

void APU::write(Word address, Byte value) {
    // NR52 (0xFF26) - Control Maestro
    if (address == 0xFF26) {
        soundEnabled = ((value >> 7) & 1) == 1;
        if (!soundEnabled) {
            channel1On = false;
        }
        return;
    }
    
    if (!soundEnabled) return;

    switch (address) {
        case 0xFF10: NR10 = value; break;
        case 0xFF11: NR11 = value; break;
        case 0xFF12: NR12 = value; break;
        case 0xFF13: 
            NR13 = value; 
            // Combinar 8 bits bajos
            frequency1 = (frequency1 & 0x0700) | NR13;
            break;
        case 0xFF14: 
            NR14 = value; 
            // Combinar 3 bits altos (enmascarados con 0x07)
            frequency1 = (frequency1 & 0x00FF) | ((NR14 & 0x07) << 8);
            
            // --- TRIGGER (Bit 7) ---
            if ((value & 0x80) != 0) {
                channel1On = true;
                int vol = (NR12 >> 4) & 0x0F;
                volume1 = vol / 15.0f; // [0.0,1.0]

                envelopeDirection1 = (NR12 & 0x08) != 0 ? 1 : -1; // Bit 3
                envelopePeriod1 = NR12 & 0x07;                    // Bits 2-0

                if (envelopePeriod1 != 0) {
                    envelopeTimer1 = envelopePeriod1 * 65536;
                }
                
                // Calculamos el timer inicial según la fórmula del procesador
                timer1 = (2048 - frequency1) * 4;
                //std::cout << "[APU] Canal 1 Disparado! Freq: " << frequency1 << " Vol: " << vol << "\n";
            }
            break;
    }
}

void APU::step(int cycles) {
    if (!soundEnabled) return;

    // 1. Actualizar el temporizador de la onda cuadrada
    timer1 -= cycles;
    if (timer1 <= 0) {
        timer1 += (2048 - frequency1) * 4;
        dutyPointer1 = (dutyPointer1 + 1) % 8;
    }

    // 1.5. Actualizar la Envolvente de Volumen
    if (envelopePeriod1 > 0) {
        envelopeTimer1 -= cycles;
        if (envelopeTimer1 <= 0) {
            // Recargar el temporizador
            envelopeTimer1 += envelopePeriod1 * 65536;
            
            // Subir o bajar el volumen
            int newVol = currentVolume1 + envelopeDirection1;
            
            // Asegurarnos de que no pase de 15 ni baje de 0
            if (newVol >= 0 && newVol <= 15) {
                currentVolume1 = newVol;
                volume1 = currentVolume1 / 15.0f;
            } else {
                // Si llegamos al límite (0 o 15), la envolvente se apaga
                envelopePeriod1 = 0; 
            }
        }
    }

    // 2. Generar muestras a 44.1 kHz
    sampleCounter += cycles;
    const int CYCLES_PER_SAMPLE = 4194304 / 44100;

    while (sampleCounter >= CYCLES_PER_SAMPLE) {
        sampleCounter -= CYCLES_PER_SAMPLE;

        float sample = 0.0f;

        // Si el canal está encendido y tiene volumen, calculamos su amplitud
        if (channel1On && volume1 > 0.0f) {
            // Leer los 2 bits más altos de NR11 para saber qué Duty Cycle usar (0, 1, 2 o 3)
            int dutyIndex = (NR11 >> 6) & 0x03;
            
            // Obtener el valor de la onda (-1.0 o 1.0) y multiplicarlo por el volumen
            sample = dutyCycles[dutyIndex][dutyPointer1] * volume1;
            sample *= 0.1f;
        }

        // Insertar canal Izquierdo y Derecho (Estéreo)
        audioBuffer.push_back(sample);
        audioBuffer.push_back(sample);
    }

    // 3. Enviar a SDL2 cuando tengamos 1024 muestras estéreo (2048 flotantes)
    if (audioBuffer.size() >= 2048) {
        const Uint32 MAX_AUDIO_QUEUE_BYTES = 35280;

        if (SDL_GetQueuedAudioSize(audioDevice) < MAX_AUDIO_QUEUE_BYTES) {
            SDL_QueueAudio(audioDevice, audioBuffer.data(), audioBuffer.size() * sizeof(float));
        }
        
        audioBuffer.clear();
    }
}