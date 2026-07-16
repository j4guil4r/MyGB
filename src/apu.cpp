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

    sweepTimer1 = 0;
    sweepPeriod1 = 0;
    sweepDirection1 = 0;
    sweepShift1 = 0;
    sweepEnabled1 = false;
    shadowFrequency1 = 0;

    lengthTimer1 = 0;
    lengthEnabled1 = false;
    lengthCounterTick1 = 0;

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
        case 0xFF16: return NR21 | 0x3F; // Solo los 2 bits de Duty son leíbles
        case 0xFF17: return NR22;
        case 0xFF18: return 0xFF;        // Frecuencia baja es Write-Only
        case 0xFF19: return NR24 | 0xBF; // Solo el bit de Length es leíble
        case 0xFF24: return NR50; 
        case 0xFF25: return NR51;
        case 0xFF1A: return NR30 | 0x7F; // Solo Bit 7 es leíble
        case 0xFF1B: return 0xFF;        // Write-only
        case 0xFF1C: return NR32 | 0x9F; // Solo Bits 5 y 6 son leíbles
        case 0xFF1D: return 0xFF;        // Write-only
        case 0xFF1E: return NR34 | 0xBF; // Solo Bit 6 es leíble
        case 0xFF20: return 0xFF;        // Write-only
        case 0xFF21: return NR42;
        case 0xFF22: return NR43;
        case 0xFF23: return NR44 | 0xBF; // Solo el bit 6 es leíble
        case 0xFF26: {
            Byte status = 0x70;
            if (soundEnabled) status |= 0x80;
            if (channel1On) status |= 0x01;
            if (channel2On) status |= 0x02;
            if (channel3On) status |= 0x04;
            if (channel4On) status |= 0x08;
            return status;
        }
        default: return 0xFF;
    }
}

void APU::write(Word address, Byte value) {
    // NR52 (0xFF26) - Control Maestro
    if (address == 0xFF26) {
        soundEnabled = ((value >> 7) & 1) == 1;
        if (!soundEnabled) {
            channel1On = false;
            channel2On = false;
            channel3On = false;
            channel4On = false;
        }
        return;
    }
    
    if (!soundEnabled) return;

    if (address >= 0xFF30 && address <= 0xFF3F) {
        waveRam[address - 0xFF30] = value;
        return;
    }

    switch (address) {
        case 0xFF10: NR10 = value; break;
        case 0xFF11: 
            NR11 = value; 
            lengthTimer1 = 64 - (value & 0x3F);
            break;
        case 0xFF12: 
            NR12 = value;
            if ((NR12 & 0xF8) == 0) {
                if (channel1On) channel1On = false;
            }
            break;
        case 0xFF13: 
            NR13 = value; 
            // Combinar 8 bits bajos
            frequency1 = (frequency1 & 0x0700) | NR13;
            break;
        case 0xFF14: 
            NR14 = value; 
            // Combinar 3 bits altos (enmascarados con 0x07)
            frequency1 = (frequency1 & 0x00FF) | ((NR14 & 0x07) << 8);

            lengthEnabled1 = (value & 0x40) != 0;
            
            // --- TRIGGER (Bit 7) ---
            if ((value & 0x80) != 0) {
                channel1On = ((NR12 & 0xF8) != 0);
                
                if (lengthTimer1 == 0) lengthTimer1 = 64;

                currentVolume1 = (NR12 >> 4) & 0x0F;
                volume1 = currentVolume1/15.0f; // [0.0,1.0]

                envelopeDirection1 = (NR12 & 0x08) != 0 ? 1 : -1; // Bit 3
                envelopePeriod1 = NR12 & 0x07;                    // Bits 2-0

                if (envelopePeriod1 != 0) {
                    envelopeTimer1 = envelopePeriod1 * 65536;
                }

                // 3. Configurar el Barrido (Sweep) leyendo NR10
                shadowFrequency1 = frequency1;
                sweepPeriod1 = (NR10 >> 4) & 0x07;
                sweepDirection1 = (NR10 & 0x08) != 0 ? -1 : 1; // 0 = Suma (Sube tono), 1 = Resta (Baja tono)
                sweepShift1 = NR10 & 0x07;
                
                // Un tick de Sweep ocurre a 128Hz, que equivale a 32,768 ciclos de CPU.
                // Si el periodo es 0, el manual indica que el temporizador actúa como si fuera 8.
                sweepTimer1 = sweepPeriod1 > 0 ? sweepPeriod1 * 32768 : 8 * 32768;
                sweepEnabled1 = (sweepPeriod1 > 0 || sweepShift1 > 0);
                
                // Calculamos el timer inicial según la fórmula del procesador
                timer1 = (2048 - frequency1) * 4;
            }
            break;
        // --- CANAL 2 ---
        case 0xFF16: 
            NR21 = value; 
            lengthTimer2 = 64 - (value & 0x3F);
            break;
        case 0xFF17: 
            NR22 = value;
            if ((NR22 & 0xF8) == 0) {
                channel2On = false;
            }
            break;
        case 0xFF18: 
            NR23 = value; 
            frequency2 = (frequency2 & 0x0700) | NR23;
            break;
        case 0xFF19: 
            NR24 = value; 
            frequency2 = (frequency2 & 0x00FF) | ((NR24 & 0x07) << 8);

            lengthEnabled2 = (value & 0x40) != 0;
            
            // --- TRIGGER CANAL 2 (Bit 7) ---
            if ((value & 0x80) != 0) {
                channel2On = ((NR22 & 0xF8) != 0);
                
                if (lengthTimer2 == 0) lengthTimer2 = 64;

                currentVolume2 = (NR22 >> 4) & 0x0F; 
                volume2 = currentVolume2 / 15.0f;    

                envelopeDirection2 = (NR22 & 0x08) != 0 ? 1 : -1;
                envelopePeriod2 = NR22 & 0x07;

                if (envelopePeriod2 != 0) {
                    envelopeTimer2 = envelopePeriod2 * 65536;
                }
                
                timer2 = (2048 - frequency2) * 4;
            }
            break;

        case 0xFF1A: 
            NR30 = value; 
            // Si apagan el DAC del Canal 3, se silencia de inmediato
            if ((NR30 & 0x80) == 0) channel3On = false;
            break;
        case 0xFF1B: 
            NR31 = value; 
            lengthTimer3 = 256 - value;
            break;
        case 0xFF1C: 
            NR32 = value; 
            break;
        case 0xFF1D: 
            NR33 = value; 
            frequency3 = (frequency3 & 0x0700) | NR33;
            break;
        case 0xFF1E: 
            NR34 = value; 
            frequency3 = (frequency3 & 0x00FF) | ((NR34 & 0x07) << 8);
            lengthEnabled3 = (value & 0x40) != 0;

            // --- TRIGGER CANAL 3 (Bit 7) ---
            if ((value & 0x80) != 0) {
                // Validación del DAC del Canal 3
                channel3On = ((NR30 & 0x80) != 0);
                
                if (lengthTimer3 == 0) lengthTimer3 = 256;
                
                // Reiniciamos el puntero de la muestra
                wavePointer = 0; 

                timer3 = (2048 - frequency3) * 2; 
            }
            break;
        // --- ESCRITURA CH4 ---
        case 0xFF20: 
            NR41 = value; 
            lengthTimer4 = 64 - (value & 0x3F);
            break;
        case 0xFF21: 
            NR42 = value;
            if ((NR42 & 0xF8) == 0) channel4On = false;
            break;
        case 0xFF22: 
            NR43 = value; 
            break;
        case 0xFF23: 
            NR44 = value; 
            lengthEnabled4 = (value & 0x40) != 0;
            
            // --- TRIGGER CANAL 4 (Bit 7) ---
            if ((value & 0x80) != 0) {
                channel4On = ((NR42 & 0xF8) != 0);
                
                if (lengthTimer4 == 0) lengthTimer4 = 64;

                currentVolume4 = (NR42 >> 4) & 0x0F;
                volume4 = currentVolume4 / 15.0f;

                envelopeDirection4 = (NR42 & 0x08) != 0 ? 1 : -1;
                envelopePeriod4 = NR42 & 0x07;

                if (envelopePeriod4 != 0) {
                    envelopeTimer4 = envelopePeriod4 * 65536;
                }
                
                // Al disparar el canal, el LFSR se reinicia a 15 bits en 1
                lfsr = 0x7FFF; 

                // Configurar el timer inicial según la fórmula del manual
                int divisorCode = NR43 & 0x07;
                int shift = (NR43 >> 4) & 0x0F;
                int divisor = (divisorCode == 0) ? 8 : (divisorCode * 16);
                timer4 = divisor << shift;
            }
            break;
        case 0xFF24: NR50 = value; break;
        case 0xFF25: NR51 = value; break;
    }
}

void APU::step(int cycles) {
    if (!soundEnabled) return;

    // ==========================================
    // 1. ACTUALIZACIÓN DE TEMPORIZADORES (FRECUENCIA)
    // ==========================================
    
    // Canal 1
    timer1 -= cycles;
    if (timer1 <= 0) {
        timer1 += (2048 - frequency1) * 4;
        dutyPointer1 = (dutyPointer1 + 1) % 8;
    }

    // Canal 2
    timer2 -= cycles;
    if (timer2 <= 0) {
        timer2 += (2048 - frequency2) * 4;
        dutyPointer2 = (dutyPointer2 + 1) % 8;
    }

    timer3 -= cycles;
    if (timer3 <= 0) {
        timer3 += (2048 - frequency3) * 2;
        wavePointer = (wavePointer + 1) % 32;
    }

    timer4 -= cycles;
    if (timer4 <= 0) {
        int divisorCode = NR43 & 0x07;
        int shift = (NR43 >> 4) & 0x0F;
        int divisor = (divisorCode == 0) ? 8 : (divisorCode * 16);
        timer4 += divisor << shift;

        // --- ALGORITMO LFSR PARA EL RUIDO ---
        // Hacemos un XOR entre el Bit 0 y el Bit 1
        int xorBit = (lfsr & 1) ^ ((lfsr >> 1) & 1);
        
        lfsr >>= 1; // Desplazamos todo a la derecha
        
        // Insertamos el resultado en el Bit 14
        lfsr |= (xorBit << 14);
        
        // Si el Bit 3 de NR43 está encendido, es el "Modo 7 bits" (Hace sonidos más metálicos)
        if ((NR43 & 0x08) != 0) {
            lfsr &= ~0x40;          // Apagamos el Bit 6 temporalmente
            lfsr |= (xorBit << 6);  // Y le insertamos el resultado
        }
    }

    // ==========================================
    // 2. ACTUALIZACIÓN DE ENVOLVENTES DE VOLUMEN (64 Hz)
    // ==========================================

    // Canal 1
    if (envelopePeriod1 > 0) {
        envelopeTimer1 -= cycles;
        if (envelopeTimer1 <= 0) {
            envelopeTimer1 += envelopePeriod1 * 65536;
            int newVol = currentVolume1 + envelopeDirection1;
            if (newVol >= 0 && newVol <= 15) {
                currentVolume1 = newVol;
                volume1 = currentVolume1 / 15.0f;
            } else {
                envelopePeriod1 = 0; 
            }
        }
    }

    // Canal 2
    if (envelopePeriod2 > 0) {
        envelopeTimer2 -= cycles;
        if (envelopeTimer2 <= 0) {
            envelopeTimer2 += envelopePeriod2 * 65536;
            int newVol = currentVolume2 + envelopeDirection2;
            if (newVol >= 0 && newVol <= 15) {
                currentVolume2 = newVol;
                volume2 = currentVolume2 / 15.0f;
            } else {
                envelopePeriod2 = 0; 
            }
        }
    }

    if (envelopePeriod4 > 0) {
        envelopeTimer4 -= cycles;
        if (envelopeTimer4 <= 0) {
            envelopeTimer4 += envelopePeriod4 * 65536;
            int newVol = currentVolume4 + envelopeDirection4;
            if (newVol >= 0 && newVol <= 15) {
                currentVolume4 = newVol;
                volume4 = currentVolume4 / 15.0f;
            } else {
                envelopePeriod4 = 0; 
            }
        }
    }

    // ==========================================
    // 3. ACTUALIZACIÓN DE SWEEP (SOLO CANAL 1)
    // ==========================================
    if (sweepTimer1 > 0) {
        sweepTimer1 -= cycles;
        if (sweepTimer1 <= 0) {
            int period = sweepPeriod1 > 0 ? sweepPeriod1 : 8;
            sweepTimer1 += period * 32768;

            if (sweepEnabled1 && sweepShift1 > 0) {
                int newFreq = shadowFrequency1 + (sweepDirection1 * (shadowFrequency1 >> sweepShift1));
                
                if (newFreq > 2047) {
                    channel1On = false;
                } 
                else if (sweepPeriod1 > 0) {
                    shadowFrequency1 = newFreq;
                    frequency1 = newFreq;
                    NR13 = frequency1 & 0xFF;
                    NR14 = (NR14 & 0xF8) | ((frequency1 >> 8) & 0x07);
                }
            }
        }
    }

    // ==========================================
    // 4. ACTUALIZACIÓN DE LENGTH TIMERS (256 Hz)
    // ==========================================

    // Canal 1
    lengthCounterTick1 += cycles;
    if (lengthCounterTick1 >= 16384) {
        lengthCounterTick1 -= 16384;
        if (lengthEnabled1 && lengthTimer1 > 0) {
            lengthTimer1--;
            if (lengthTimer1 == 0) channel1On = false; 
        }
    }

    // Canal 2
    lengthCounterTick2 += cycles;
    if (lengthCounterTick2 >= 16384) {
        lengthCounterTick2 -= 16384;
        if (lengthEnabled2 && lengthTimer2 > 0) {
            lengthTimer2--;
            if (lengthTimer2 == 0) channel2On = false; 
        }
    }

    lengthCounterTick3 += cycles;
    if (lengthCounterTick3 >= 16384) {
        lengthCounterTick3 -= 16384;
        if (lengthEnabled3 && lengthTimer3 > 0) {
            lengthTimer3--;
            if (lengthTimer3 == 0) channel3On = false; 
        }
    }

    lengthCounterTick4 += cycles;
    if (lengthCounterTick4 >= 16384) {
        lengthCounterTick4 -= 16384;
        if (lengthEnabled4 && lengthTimer4 > 0) {
            lengthTimer4--;
            if (lengthTimer4 == 0) channel4On = false; 
        }
    }

    // ==========================================
    // 5. GENERACIÓN Y MEZCLA DE MUESTRAS (MIXER)
    // ==========================================
    sampleCounter += cycles;
    const int CYCLES_PER_SAMPLE = 4194304 / 44100;

    while (sampleCounter >= CYCLES_PER_SAMPLE) {
        sampleCounter -= CYCLES_PER_SAMPLE;

        float sample1 = 0.0f;
        float sample2 = 0.0f;
        float sample3 = 0.0f;
        float sample4 = 0.0f;

        // Muestra del Canal 1
        if (channel1On && volume1 > 0.0f) {
            int dutyIndex1 = (NR11 >> 6) & 0x03;
            sample1 = dutyCycles[dutyIndex1][dutyPointer1] * volume1;
        }

        // Muestra del Canal 2
        if (channel2On && volume2 > 0.0f) {
            int dutyIndex2 = (NR21 >> 6) & 0x03;
            sample2 = dutyCycles[dutyIndex2][dutyPointer2] * volume2;
        }

        if (channel3On && (NR30 & 0x80)) {
            Byte waveByte = waveRam[wavePointer/2];
            int nibble = (wavePointer % 2 == 0) ? (waveByte >> 4):(waveByte & 0x0F);
            int volumeCode = (NR32 >> 5) & 0x03;
            int shiftedNibble = 0;

            switch (volumeCode) {
                case 0: shiftedNibble = 0; break;           // Mute (0%)
                case 1: shiftedNibble = nibble; break;      // 100% (Muestra intacta)
                case 2: shiftedNibble = nibble >> 1; break; // 50% (Desplazamos 1 bit a la derecha)
                case 3: shiftedNibble = nibble >> 2; break; // 25% (Desplazamos 2 bits)
            }

            // El nibble va de 0 a 15. Lo normalizamos a un rango de -1.0 a 1.0 para el audio flotante.
            sample3 = (shiftedNibble/7.5f) - 1.0f;
        }

        if (channel4On && volume4 > 0.0f) {
            // Si el bit 0 es 0, la señal es Alta. Si es 1, la señal es Baja.
            sample4 = ((lfsr & 1) == 0 ? 1.0f : -1.0f) * volume4;
        }

        // ==========================================
        // 5.1 DISTRIBUCIÓN ESTÉREO (PANNING - NR51)
        // ==========================================
        float leftMix = 0.0f;
        float rightMix = 0.0f;

        // Canal 1
        if (NR51 & 0x10) leftMix += sample1; 
        if (NR51 & 0x01) rightMix += sample1; 
        // Canal 2
        if (NR51 & 0x20) leftMix += sample2;  
        if (NR51 & 0x02) rightMix += sample2; 
        // Canal 3
        if (NR51 & 0x40) leftMix += sample3;  
        if (NR51 & 0x04) rightMix += sample3; 
        // Canal 4
        if (NR51 & 0x80) leftMix += sample4;  
        if (NR51 & 0x08) rightMix += sample4; 

        // ==========================================
        // 5.2 VOLUMEN MAESTRO (FADE-OUT - NR50)
        // ==========================================
        
        float masterLeftVol = ((NR50 >> 4) & 0x07) / 7.0f;  
        float masterRightVol = (NR50 & 0x07) / 7.0f;

        float finalLeft = leftMix * masterLeftVol * 0.1f;
        float finalRight = rightMix * masterRightVol * 0.1f;

        audioBuffer.push_back(finalLeft);
        audioBuffer.push_back(finalRight);
    }

    // ==========================================
    // 6. ENVÍO A SDL2
    // ==========================================
    if (audioBuffer.size() >= 1024) {
        const Uint32 MAX_AUDIO_QUEUE_BYTES = 16384;
        if (SDL_GetQueuedAudioSize(audioDevice) < MAX_AUDIO_QUEUE_BYTES) {
            SDL_QueueAudio(audioDevice, audioBuffer.data(), audioBuffer.size() * sizeof(float));
        }
        audioBuffer.clear();
    }
}