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

    // --- CANAL 2: Onda Cuadrada (Sin Barrido) ---
    Byte NR21; // 0xFF16 - Duty Cycle y Longitud
    Byte NR22; // 0xFF17 - Volumen y Envolvente
    Byte NR23; // 0xFF18 - Frecuencia (8 bits bajos)
    Byte NR24; // 0xFF19 - Frecuencia (3 bits altos) y Trigger

    int timer2 = 0;         
    int dutyPointer2 = 0;   
    int frequency2 = 0;     
    float volume2 = 0.0f;      
    bool channel2On;    

    // --- Variables de la Envolvente CH2 ---
    int currentVolume2 = 0;
    int envelopeTimer2 = 0;
    int envelopePeriod2 = 0;
    int envelopeDirection2 = 1;

    // --- Variables de Longitud CH2 ---
    int lengthTimer2 = 0;
    bool lengthEnabled2 = false;
    int lengthCounterTick2 = 0;

    // --- CANAL 3: Wave Channel (Forma de Onda Personalizada) ---
    Byte NR30; // 0xFF1A - DAC Power (Encendido/Apagado)
    Byte NR31; // 0xFF1B - Longitud de onda
    Byte NR32; // 0xFF1C - Nivel de Volumen
    Byte NR33; // 0xFF1D - Frecuencia (8 bits bajos)
    Byte NR34; // 0xFF1E - Frecuencia (3 bits altos) y Trigger

    bool channel3On;
    int timer3 = 0;
    int frequency3 = 0;
    
    // La Wave RAM tiene 16 bytes, pero cada byte tiene 2 muestras de 4 bits (nibbles).
    // Por lo tanto, hay 32 muestras en total.
    int wavePointer = 0; 
    Byte waveRam[16] = {0}; // Memoria que va de 0xFF30 a 0xFF3F

    // Variables de Longitud CH3
    int lengthTimer3 = 0;
    bool lengthEnabled3 = false;
    int lengthCounterTick3 = 0;

    // --- CANAL 4: Ruido (Noise Channel) ---
    Byte NR41; // 0xFF20 - Longitud
    Byte NR42; // 0xFF21 - Volumen y Envolvente
    Byte NR43; // 0xFF22 - Polinomio (Frecuencia y Aleatoriedad)
    Byte NR44; // 0xFF23 - Trigger y Control de Longitud

    bool channel4On;
    int timer4 = 0;
    int lfsr = 0x7FFF; // Se inicializa con todos los 15 bits encendidos
    float volume4 = 0.0f;

    // --- Variables de la Envolvente CH4 ---
    int currentVolume4 = 0;
    int envelopeTimer4 = 0;
    int envelopePeriod4 = 0;
    int envelopeDirection4 = 1;

    // --- Variables de Longitud CH4 ---
    int lengthTimer4 = 0;
    bool lengthEnabled4 = false;
    int lengthCounterTick4 = 0;

    // Variables de Control Maestro
    Byte NR50;
    Byte NR51;

    // Patrones fijos de onda para los 4 Duty Cycles de la Game Boy (8 pasos cada uno)
    const float dutyCycles[4][8] = {
        {-1, -1, -1, -1, -1, -1, -1,  1}, // 12.5%
        { 1, -1, -1, -1, -1, -1, -1,  1}, // 25%
        { 1, -1, -1, -1, -1,  1,  1,  1}, // 50%
        {-1,  1,  1,  1,  1,  1,  1, -1}  // 75%
    };

    // helpers
    void tickEnvelope(int& period, int& timer, int& currentVol, float& volFloat, int direction);
    void tickLength(bool enabled, int& timer, bool& channelOn);
    void tickSweep();

    // --- FRAME SEQUENCER ---
    int frameSequencerTimer = 8192;
    int frameSequencerStep = 0;

    bool sweepHasCalculatedWithNegate = false;

public:
    APU();
    ~APU();

    Byte read(Word address) const;
    void write(Word address, Byte value);
    void resetSequencerPhase();

    void step(int cycles);
};