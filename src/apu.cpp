#include "apu.h"
#include <iostream>

static SDL_AudioDeviceID audioDevice = 0;

APU::APU(){
    soundEnabled = true;
    channel1On = true;
    channel2On = false;
    channel3On = false;
    channel4On = false;
    sampleCounter = 0;

    NR10 = 0x80; NR11 = 0xBF; NR12 = 0xF3; NR13 = 0xFF; NR14 = 0xBF;
    NR21 = 0x3F; NR22 = 0x00; NR23 = 0xFF; NR24 = 0xBF;
    NR30 = 0x7F; NR31 = 0xFF; NR32 = 0x9F; NR33 = 0xFF; NR34 = 0xBF;
    NR41 = 0xFF; NR42 = 0x00; NR43 = 0x00; NR44 = 0xBF;
    NR50 = 0x77; NR51 = 0xF3;
    
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

void APU::resetSequencerPhase() {
    frameSequencerTimer = 8192;
    //frameSequencerStep = 0;
}

void APU::tickEnvelope(int& period, int& timer, int& currentVol, float& volFloat, int direction) {
    if (period > 0) {
        timer--;
        if (timer <= 0) {
            timer = period; // Recargamos el timer con el periodo real
            int newVol = currentVol + direction;
            if (newVol >= 0 && newVol <= 15) {
                currentVol = newVol;
                volFloat = currentVol / 15.0f;
            } else {
                period = 0; // Apagamos la envolvente
            }
        }
    }
}

void APU::tickLength(bool enabled, int& timer, bool& channelOn) {
    if (enabled && timer > 0) {
        timer--;
        if (timer == 0) channelOn = false; 
    }
}

void APU::tickSweep() {
    sweepTimer1--;

    if (sweepTimer1 <= 0) {
        // LEEMOS EL PERIODO DINÁMICAMENTE DIRECTO DEL REGISTRO
        int currentPeriod = (NR10 >> 4) & 0x07;
        
        // El temporizador se recarga con el periodo actual
        sweepTimer1 = currentPeriod > 0 ? currentPeriod : 8;

        // Evaluamos usando la bandera guardada, pero con el periodo actual
        if (sweepEnabled1 && currentPeriod > 0) {
            
            // Leemos Shift y Dirección dinámicamente
            int currentShift = NR10 & 0x07;
            int currentDir = (NR10 & 0x08) != 0 ? -1 : 1;

            if (currentDir == -1) {
                sweepHasCalculatedWithNegate = true;
            }

            int shiftAmount = shadowFrequency1 >> currentShift;
            int calculatedFreq = shadowFrequency1 + (currentDir * shiftAmount);

            // Verificación de Overflow principal
            if (calculatedFreq > 2047) {
                channel1On = false; 
            } 
            else if (currentShift > 0) {
                // Aplicar la nueva frecuencia
                shadowFrequency1 = calculatedFreq;
                frequency1 = calculatedFreq;

                NR13 = calculatedFreq & 0xFF;
                NR14 = (NR14 & 0xF8) | ((calculatedFreq >> 8) & 0x07);

                // Súper Quirk: Segunda predicción inmediata
                int nextShiftAmount = shadowFrequency1 >> currentShift;
                int nextCalculatedFreq = shadowFrequency1 + (currentDir * nextShiftAmount);
                if (nextCalculatedFreq > 2047) {
                    channel1On = false;
                }
            }
        }
    }
}

Byte APU::read(Word address) const {
    if (address == 0xFF26) {
        Byte res = 0x70;
        if (soundEnabled) res |= 0x80;
        if (channel4On) res |= 0x08;
        if (channel3On) res |= 0x04;
        if (channel2On) res |= 0x02;
        if (channel1On) res |= 0x01;
        return res;
    }

    if (address >= 0xFF30 && address <= 0xFF3F) {
        return waveRam[address - 0xFF30];
    }

    switch (address) {
        // --- Canal 1 ---
        case 0xFF10: return NR10 | 0x80;
        case 0xFF11: return NR11 | 0x3F;
        case 0xFF12: return NR12 | 0x00;
        case 0xFF13: return 0xFF;       
        case 0xFF14: return NR14 | 0xBF;
        
        // --- Canal 2 ---
        case 0xFF15: return 0xFF;
        case 0xFF16: return NR21 | 0x3F;
        case 0xFF17: return NR22 | 0x00;
        case 0xFF18: return 0xFF;       
        case 0xFF19: return NR24 | 0xBF;
        
        // --- Canal 3 ---
        case 0xFF1A: return NR30 | 0x7F;
        case 0xFF1B: return 0xFF;
        case 0xFF1C: return NR32 | 0x9F;
        case 0xFF1D: return 0xFF;
        case 0xFF1E: return NR34 | 0xBF;
        
        // --- Canal 4 ---
        case 0xFF1F: return 0xFF;
        case 0xFF20: return 0xFF;
        case 0xFF21: return NR42 | 0x00;
        case 0xFF22: return NR43 | 0x00;
        case 0xFF23: return NR44 | 0xBF;
        
        // --- Control de Paneo y Volumen ---
        case 0xFF24: return NR50 | 0x00;
        case 0xFF25: return NR51 | 0x00;
        default: return 0xFF;
    }
}

void APU::write(Word address, Byte value) {
    // NR52 (0xFF26) - Control Maestro
    if (!soundEnabled && address != 0xFF26) {
        if (address == 0xFF11) { lengthTimer1 = 64 - (value & 0x3F); return; }
        if (address == 0xFF16) { lengthTimer2 = 64 - (value & 0x3F); return; }
        if (address == 0xFF1B) { lengthTimer3 = 256 - value; return; }
        if (address == 0xFF20) { lengthTimer4 = 64 - (value & 0x3F); return; }
        return;
    }
    if (address == 0xFF26) {
        bool turningOn = (value & 0x80) != 0;

        if (!soundEnabled && turningOn) {
            frameSequencerStep = 0;
        }

        if (soundEnabled && !turningOn) {

            NR10 = 0; NR11 = 0; NR12 = 0; NR13 = 0; NR14 = 0;
            NR21 = 0; NR22 = 0; NR23 = 0; NR24 = 0;
            NR30 = 0; NR31 = 0; NR32 = 0; NR33 = 0; NR34 = 0;
            NR41 = 0; NR42 = 0; NR43 = 0; NR44 = 0;
            NR50 = 0; NR51 = 0;

            channel1On = false;
            channel2On = false;
            channel3On = false;
            channel4On = false;
        }
        
        soundEnabled = turningOn;
        return; 
    }
    if (address >= 0xFF30 && address <= 0xFF3F) {
        waveRam[address - 0xFF30] = value;
        return;
    }

    switch (address) {
        case 0xFF10: {
            bool wasNegate = (NR10 & 0x08) != 0;
            bool isNegate = (value & 0x08) != 0;
            
            NR10 = value; 

            if (wasNegate && !isNegate && sweepHasCalculatedWithNegate) {
                channel1On = false;
            }
            break;
        }
        case 0xFF11: 
            NR11 = value; 
            lengthTimer1 = 64 - (value & 0x3F);
            break;
        case 0xFF12: 
            NR12 = value;
            if ((NR12 & 0xF8) == 0) {
                channel1On = false;
            }
            break;
        case 0xFF13: 
            NR13 = value; 
            // Combinar 8 bits bajos
            frequency1 = (frequency1 & 0x0700) | NR13;
            break;
        case 0xFF14:{
            bool wasEnabled = (NR14 & 0x40) != 0;
            bool isEnabled = (value & 0x40) != 0;
            bool trigger = (value & 0x80) != 0;

            NR14 = value; 
            frequency1 = (frequency1 & 0x00FF) | ((NR14 & 0x07) << 8);
            lengthEnabled1 = isEnabled;

            bool isFirstHalf = (frameSequencerStep % 2 == 0);

            if (!wasEnabled && isEnabled && !isFirstHalf) {
                if (lengthTimer1 > 0) {
                    lengthTimer1--;
                    if (lengthTimer1 == 0 && !trigger) {
                        channel1On = false;
                    }
                }
            }
            
            
            // --- TRIGGER (Bit 7) ---
            if (trigger) {
                channel1On = ((NR12 & 0xF8) != 0);

                if (lengthTimer1 == 0) {
                    lengthTimer1 = 64;
                    if (isEnabled && !isFirstHalf) {
                        lengthTimer1 = 63;
                    }
                }

                currentVolume1 = (NR12 >> 4) & 0x0F;
                volume1 = currentVolume1/15.0f; // [0.0,1.0]

                envelopeDirection1 = (NR12 & 0x08) != 0 ? 1 : -1; // Bit 3
                envelopePeriod1 = NR12 & 0x07;                    // Bits 2-0

                /*if (envelopePeriod1 != 0) {
                    envelopeTimer1 = envelopePeriod1 * 65536;
                }*/
                envelopeTimer1 = envelopePeriod1;

                // 3. Configurar el Barrido (Sweep) leyendo NR10
                shadowFrequency1 = frequency1;
                sweepPeriod1 = (NR10 >> 4) & 0x07;
                sweepDirection1 = (NR10 & 0x08) != 0 ? -1 : 1; // 0 = Suma (Sube tono), 1 = Resta (Baja tono)
                sweepShift1 = NR10 & 0x07;
                
                // Un tick de Sweep ocurre a 128Hz, que equivale a 32,768 ciclos de CPU.
                // Si el periodo es 0, el manual indica que el temporizador actúa como si fuera 8.
                //sweepTimer1 = sweepPeriod1 > 0 ? sweepPeriod1 * 32768 : 8 * 32768;
                sweepTimer1 = sweepPeriod1 > 0 ? sweepPeriod1 : 8;
                sweepEnabled1 = (sweepPeriod1 > 0 || sweepShift1 > 0);

                sweepHasCalculatedWithNegate = false;

                // --- SOLUCIÓN ERROR 2: CÁLCULO DE OVERFLOW EN EL TRIGGER ---
                if (sweepShift1 > 0) {
                    if (sweepDirection1 == -1) {
                        sweepHasCalculatedWithNegate = true;
                    }
                    int shiftAmount = shadowFrequency1 >> sweepShift1;
                    int calculatedFreq = shadowFrequency1 + (sweepDirection1 * shiftAmount);
                    
                    // Si el cálculo supera el máximo permitido (2047), el canal muere
                    if (calculatedFreq > 2047) {
                        channel1On = false;
                    }
                }

                timer1 = (2048 - frequency1) * 4;
            }
            break;
        }
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
        case 0xFF19:{
            bool wasEnabled = (NR24 & 0x40) != 0;
            bool isEnabled = (value & 0x40) != 0;
            bool trigger = (value & 0x80) != 0;

            NR24 = value; 
            frequency2 = (frequency2 & 0x00FF) | ((NR24 & 0x07) << 8);
            lengthEnabled2 = (value & 0x40) != 0;

            bool isFirstHalf = (frameSequencerStep % 2 == 0);

            if (!wasEnabled && isEnabled && !isFirstHalf) {
                if (lengthTimer2 > 0) {
                    lengthTimer2--;
                    if (lengthTimer2 == 0 && !trigger) {
                        channel2On = false;
                    }
                }
            }
            
            
            // --- TRIGGER CANAL 2 (Bit 7) ---
            if (trigger) {
                channel2On = ((NR22 & 0xF8) != 0);

                if (lengthTimer2 == 0) {
                    lengthTimer2 = 64;
                    if (isEnabled && !isFirstHalf) {
                        lengthTimer2 = 63;
                    }
                }

                currentVolume2 = (NR22 >> 4) & 0x0F; 
                volume2 = currentVolume2 / 15.0f;    

                envelopeDirection2 = (NR22 & 0x08) != 0 ? 1 : -1;
                envelopePeriod2 = NR22 & 0x07;
                envelopeTimer2 = envelopePeriod2;
                
                timer2 = (2048 - frequency2) * 4;
            }
            break;
        }
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
        case 0xFF1E:{
            bool wasEnabled = (NR34 & 0x40) != 0;
            bool isEnabled = (value & 0x40) != 0;
            bool trigger = (value & 0x80) != 0;

            NR34 = value; 
            frequency3 = (frequency3 & 0x00FF) | ((NR34 & 0x07) << 8);
            lengthEnabled3 = (value & 0x40) != 0;

            bool isFirstHalf = (frameSequencerStep % 2 == 0);

            // 1. Reloj Extra SÓLO por encender la longitud en la primera mitad
            if (!wasEnabled && isEnabled && !isFirstHalf) {
                if (lengthTimer3 > 0) {
                    lengthTimer3--;
                    if (lengthTimer3 == 0 && !trigger) {
                        channel3On = false;
                    }
                }
            }

            // --- TRIGGER CANAL 3 (Bit 7) ---
            if (trigger) {
                // Validación del DAC del Canal 3
                channel3On = ((NR30 & 0x80) != 0);

                if (lengthTimer3 == 0) {
                    lengthTimer3 = 256;
                    // 2. EL SÚPER QUIRK CORREGIDO:
                    // Baja a 63 SOLO si está habilitado Y en la primera mitad
                    if (isEnabled && !isFirstHalf) {
                        lengthTimer3 = 255;
                    }
                }
                
                // Reiniciamos el puntero de la muestra
                wavePointer = 0; 

                timer3 = (2048 - frequency3) * 2; 
            }
            break;
        }
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
        case 0xFF23: {
            bool wasEnabled = (NR44 & 0x40) != 0;
            bool isEnabled = (value & 0x40) != 0;
            bool trigger = (value & 0x80) != 0;

            NR44 = value; 
            lengthEnabled4 = (value & 0x40) != 0;

            bool isFirstHalf = (frameSequencerStep % 2 == 0);

            // 1. Reloj Extra SÓLO por encender la longitud en la primera mitad
            if (!wasEnabled && isEnabled && !isFirstHalf) {
                if (lengthTimer4 > 0) {
                    lengthTimer4--;
                    if (lengthTimer4 == 0 && !trigger) {
                        channel4On = false;
                    }
                }
            }
            
            
            // --- TRIGGER CANAL 4 (Bit 7) ---
            if (trigger) {
                channel4On = ((NR42 & 0xF8) != 0);

                if (lengthTimer4 == 0) {
                    lengthTimer4 = 64;
                    // 2. EL SÚPER QUIRK CORREGIDO:
                    // Baja a 63 SOLO si está habilitado Y en la primera mitad
                    if (isEnabled && !isFirstHalf) {
                        lengthTimer4 = 63;
                    }
                }

                currentVolume4 = (NR42 >> 4) & 0x0F;
                volume4 = currentVolume4 / 15.0f;

                envelopeDirection4 = (NR42 & 0x08) != 0 ? 1 : -1;
                envelopePeriod4 = NR42 & 0x07;

                /*if (envelopePeriod4 != 0) {
                    envelopeTimer4 = envelopePeriod4 * 65536;
                }*/
               envelopeTimer4 = envelopePeriod4;
                
                // Al disparar el canal, el LFSR se reinicia a 15 bits en 1
                lfsr = 0x7FFF; 

                // Configurar el timer inicial según la fórmula del manual
                int divisorCode = NR43 & 0x07;
                int shift = (NR43 >> 4) & 0x0F;
                int divisor = (divisorCode == 0) ? 8 : (divisorCode * 16);
                timer4 = divisor << shift;
            }
            break;
        }
        case 0xFF24: NR50 = value; break;
        case 0xFF25: NR51 = value; break;
    }
    //if (address == 0xFF26 && (value & 0x80) == 0) {
    //std::cout << "[NR52] Registros conservados. NR11: 0x" << std::hex << (int)NR11 << "\n";}
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
    // FRAME SEQUENCER (512 Hz)
    // ==========================================
    frameSequencerTimer -= cycles;
    if (frameSequencerTimer <= 0) {
        frameSequencerTimer += 8192; // Reiniciamos el reloj

        switch (frameSequencerStep) {
            case 0:
            case 4:
                // 256 Hz: Reloj de Longitud
                tickLength(lengthEnabled1, lengthTimer1, channel1On);
                tickLength(lengthEnabled2, lengthTimer2, channel2On);
                tickLength(lengthEnabled3, lengthTimer3, channel3On);
                tickLength(lengthEnabled4, lengthTimer4, channel4On);
                break;
            case 2:
            case 6:
                // 256 Hz: Reloj de Longitud
                tickLength(lengthEnabled1, lengthTimer1, channel1On);
                tickLength(lengthEnabled2, lengthTimer2, channel2On);
                tickLength(lengthEnabled3, lengthTimer3, channel3On);
                tickLength(lengthEnabled4, lengthTimer4, channel4On);
                
                // 128 Hz: Reloj de Barrido (Sweep)
                tickSweep();
                break;
            case 7:
                // 64 Hz: Reloj de Envolventes de Volumen
                tickEnvelope(envelopePeriod1, envelopeTimer1, currentVolume1, volume1, envelopeDirection1);
                tickEnvelope(envelopePeriod2, envelopeTimer2, currentVolume2, volume2, envelopeDirection2);
                tickEnvelope(envelopePeriod4, envelopeTimer4, currentVolume4, volume4, envelopeDirection4);
                break;
            // Los pasos 1, 3 y 5 no hacen "tic" en ningún componente
        }
        
        // Avanzamos al siguiente paso (0 a 7)
        frameSequencerStep = (frameSequencerStep + 1) % 8;
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