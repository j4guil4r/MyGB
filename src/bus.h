#pragma once
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "types.h"

class Bus {
public:
    Bus();
    ~Bus() = default;

    Byte read(Word addr) const;
    void write(Word addr, Byte data);

    bool loadROM(const std::string& filename);

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

    // El Cartucho (Tamaño dinámico)
    std::vector<Byte> cartridgeMemory;

    // Interrupt Enable Register (FFFF)
    // Lo guardamos aparte porque es solo un byte muy importante
    Byte ieRegister = 0;
};