#pragma once
#include <cstdint>
#include <array>
#include "types.h"

// Dimensiones originales de la Game Boy
constexpr int GB_WIDTH = 160;
constexpr int GB_HEIGHT = 144;
constexpr int SCALE = 4;

class Bus;

class PPU {
public:

    // --- MEMORIA GRÁFICA ---
    std::array<Byte, 0x2000> vram{}; // 8KB de Video RAM (0x8000 - 0x9FFF)
    std::array<Byte, 0x00A0> oam{};  // 160 Bytes de Object Attribute Memory (0xFE00 - 0xFE9F)

    // Usamos uint32_t para guardar colores en formato ARGB (Alpha, Red, Green, Blue) que usa SDL2.
    std::array<uint32_t, GB_WIDTH * GB_HEIGHT> framebuffer {};
    bool frameReady = false;
    bool requestVBlankInterrupt = false;
    bool requestStatInterrupt = false;

    // --- REGISTROS DE HARDWARE (0xFF40 - 0xFF4B) ---
    Byte lcdc = 0x91; // 0xFF40 - LCD Control (Pantalla encendida por defecto)
    Byte stat = 0x85; // 0xFF41 - LCD Status
    Byte scy = 0;     // 0xFF42 - Scroll Y
    Byte scx = 0;     // 0xFF43 - Scroll X
    Byte ly = 0;      // 0xFF44 - LCD Y Coordinate | Current Scanline (0 - 143) visible - (144 - 153) VBlank
    Byte lyc = 0;     // 0xFF45 - LY Compare
    Byte dma = 0;     // 0xFF46 - DMA Transfer
    Byte bgp = 0xFC;  // 0xFF47 - Background Palette
    Byte obp0 = 0xFF; // 0xFF48 - Object Palette 0
    Byte obp1 = 0xFF; // 0xFF49 - Object Palette 1
    Byte wy = 0;      // 0xFF4A - Window Y
    Byte wx = 0;      // 0xFF4B - Window X

    // Reloj interno de la PPU
    int dots = 0;

    PPU();
    ~PPU();

    // Igual que los timers, la PPU necesita saber cuántos ciclos pasaron en la CPU
    // para saber en qué momento dibujar la pantalla.
    void step(int cpuCycles);
    void reset();

    // Funciones para que el Bus se comunique con la PPU
    Byte read(Word address) const;
    void write(Word address, Byte value);

private:
    void setMode(int mode); // Helper para cambiar los bits 0 y 1 del registro STAT
    void drawScanline();
    void drawSprites();
    void drawWindow();

    // utils:
    inline int decode2bpp(Byte lo, Byte hi, int bit);
    inline Byte readVRAM(Word addr);
    uint32_t getTilePixelColor(Byte tileNumber, Word tileDataBase, Byte line, Byte xPixel, Byte paletteReg);

    enum class PPUMode {
        HBlank = 0,
        VBlank = 1,
        OAM = 2,
        Transfer = 3
    };

    PPUMode currentMode = PPUMode::OAM;

    bool prevStatLine = false;
};