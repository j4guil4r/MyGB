#pragma once
#include "types.h"
#include <vector>
#include <SDL2/SDL.h>

class APU{
private:
    std::vector<float> audioBuffer;
    bool soundEnabled;

    // --- CANAL 1: Onda Cuadrada con Barrido ---
    
    // Registros directos de la memoria
    Byte NR10; // 0xFF10 - Sweep (Barrido)
    Byte NR11; // 0xFF11 - Duty Cycle y Longitud de onda
    Byte NR12; // 0xFF12 - Volumen y Envolvente
    Byte NR13; // 0xFF13 - Frecuencia (8 bits bajos)
    Byte NR14; // 0xFF14 - Frecuencia (3 bits altos) y Control de disparo

    // Estado interno para la síntesis de audio
    int timer1;         // Temporizador que decrece con la CPU
    int sampleCounter;  // Cuenta cicclos para enviar muestras a 44100Hz
    int dutyPointer1;   // Apunta en qué parte del ciclo de trabajo (0-7) estamos
    int frequency1;     // El valor de 11 bits combinado (NR13 y NR14)
    float volume1;      // Volumen actual del canal (0.0 a 1.0)
    bool channel1On;    // ¿Está sonando el canal?

    // --- Variables de la Envolvente ---
    int currentVolume1;
    int envelopeTimer1;
    int envelopePeriod1;
    int envelopeDirection1;

    // --- Variables del Sweep (Barrido) ---
    int sweepTimer1;
    int sweepPeriod1;
    int sweepDirection1;
    int sweepShift1;
    bool sweepEnabled1;
    int shadowFrequency1;

    // --- Variables de Longitud (Length) ---
    int lengthTimer1;
    bool lengthEnabled1;
    int lengthCounterTick1;

    // Variables de Control Maestro
    Byte NR50 = 0x00;
    Byte NR51 = 0x00;

    // Patrones fijos de onda para los 4 Duty Cycles de la Game Boy (8 pasos cada uno)
    const float dutyCycles[4][8] = {
        {-1, -1, -1, -1, -1, -1, -1,  1}, // 12.5%
        { 1, -1, -1, -1, -1, -1, -1,  1}, // 25%
        { 1, -1, -1, -1, -1,  1,  1,  1}, // 50%
        {-1,  1,  1,  1,  1,  1,  1, -1}  // 75%
    };

public:
    APU();
    ~APU();

    Byte read(Word address) const;
    void write(Word address, Byte value);

    void step(int cycles);
};