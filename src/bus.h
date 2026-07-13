#pragma once
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "types.h"
#include "ppu.h"
#include "apu.h"
#include "cartridge.h"

class Bus {
public:
    PPU ppu;
    APU apu;

    Bus();
    ~Bus() = default;

    Cartridge cartridge;    

    [[nodiscard]] Byte read(Word addr) const;
    void write(Word addr, Byte data);

    std::string getSerialOutput () const;
    void clearSerialOutput() { serialOutput = ""; }

    // --- MEMORIAS INTERNAS ---

    // VRAM (8KB) - Gráficos (Tiles y Mapas)
    // Rango: 8000 - 9FFF
    std::array<Byte, 8 * 1024> vram;

    // WRAM (8KB) - RAM de trabajo (Variables del juego)
    // Rango: C000 - DFFF
    std::array<Byte, 8 * 1024> wram;

    // OAM (160 bytes) - Memoria de Sprites
    // Rango: FE00 - FE9F (40 sprites * 4 bytes)
    std::array<Byte, 160> oam;

    // HRAM (127 bytes) - High RAM (Variables ultra rápidas)
    // Rango: FF80 - FFFE
    std::array<Byte, 127> hram;

    // Interrupt Enable Register (FFFF)
    Byte ieRegister = 0;
    Byte ifRegister = 0; // 0xFF0F - Interrupt Flag

    // --- TIMERS ---
    Byte div = 0;   // 0xFF04 - Divider Register (Incrementa siempre)
    Byte tima = 0;  // 0xFF05 - Timer Counter (El que dispara la interrupción)
    Byte tma = 0;   // 0xFF06 - Timer Modulo (Valor de recarga)
    Byte tac = 0;   // 0xFF07 - Timer Control (Velocidad y On/Off)

    // Contadores internos para gestionar la frecuencia
    // La GB corre a 4194304 Hz.
    long long divCounter = 0;   // Acumulador para el registro DIV
    long long timerCounter = 0; // Acumulador para el registro TIMA

    void updateTimers(long long cycles);

    // Lo usaremos para que el Timer le diga a la CPU "¡Oye!"
    void requestInterrupt(int bit);

    Byte ly = 0; // 0xFF44 - LCD Y Coordinate
    long long ppuCounter = 0; // Para simular el dibujo de líneas

    // --- JOYPAD ---
    // Inicializamos todo en 0x0F (puros 1s en los 4 bits bajos = nada presionado)
    Byte joypadDir = 0x0F;    // Flechas: Abajo(3), Arriba(2), Izquierda(1), Derecha(0)
    Byte joypadAction = 0x0F; // Acción: Start(3), Select(2), B(1), A(0)
    Byte joypadSelect = 0xCF; // Lo que el juego nos pide leer (Bits 4 y 5)
private:
    std::string serialOutput = "";
};