#include "ppu.h"

PPU::PPU() {
    reset();
}


PPU::~PPU(){}


void PPU::reset() {
    // Llenamos la pantalla de "blanco" (o el verde clásico de Game Boy) para empezar
    // El formato ARGB para blanco es 0xFFFFFFFF
    framebuffer.fill(0xFFFFFFFF); 
    frameReady = false;
}

// Helper para cambiar el modo sin destruir los otros bits importantes del STAT
void PPU::setMode(int mode) {
    // Apagamos los 2 bits más bajos (modo actual) y le hacemos OR con el nuevo modo
    stat = (stat & ~0x03) | (mode & 0x03);
}


void PPU::step(int cycles) {
    // Si la pantalla está apagada 
    if (!(lcdc & 0x80)) {
        dots = 0;
        ly = 0;
        currentMode = PPUMode::HBlank;
        stat = (stat & ~0x03); // Modo 0 en STAT de forma segura
        return;
    }

    dots += cycles;

    switch (currentMode) {
        case PPUMode::OAM: // Modo 2
            if (dots >= 80) {
                dots -= 80;
                currentMode = PPUMode::Transfer;
            }
            break;

        case PPUMode::Transfer: // Modo 3
            if (dots >= 172) {
                dots -= 172;
                currentMode = PPUMode::HBlank;
                
                // TODO:
                // drawScanline(); 
            }
            break;

        case PPUMode::HBlank: // Modo 0
            if (dots >= 204) {
                dots -= 204;
                ly++;

                if (ly == 144) {
                    currentMode = PPUMode::VBlank;
                    frameReady = true;
                    // TODO: Disparar Interrupción de V-Blank (INT 40)
                } else {
                    currentMode = PPUMode::OAM;
                }
            }
            break;

        case PPUMode::VBlank: // Modo 1
            if (dots >= 456) {
                dots -= 456;
                ly++;

                if (ly > 153) {
                    ly = 0;
                    currentMode = PPUMode::OAM;
                }
            }
            break;
    }

    // 1. Actualizamos de forma segura los bits 0 y 1 del STAT con el modo actual
    auto modeNum = static_cast<Byte>(currentMode);
    stat = (stat & ~0x03) | (modeNum & 0x03);

    // 2. Comprobamos flag LY == LYC (Bit 2 del STAT)
    if (ly == lyc) {
        stat |= 0x04; // Encender bit 2
        // TODO: Disparar interrupción STAT si el bit 6 está encendido
    } else {
        stat &= ~0x04; // Apagar bit 2
    }
}

Byte PPU::read(Word address) const {
    if (address >= 0x8000 && address <= 0x9FFF) return vram[address - 0x8000];
    if (address >= 0xFE00 && address <= 0xFE9F) return oam[address - 0xFE00];
    
    switch (address) {
        case 0xFF40: return lcdc;
        case 0xFF41: return stat;
        case 0xFF42: return scy;
        case 0xFF43: return scx;
        case 0xFF44: return ly;
        case 0xFF45: return lyc;
        case 0xFF46: return dma;
        case 0xFF47: return bgp;
        case 0xFF48: return obp0;
        case 0xFF49: return obp1;
        case 0xFF4A: return wy;
        case 0xFF4B: return wx;
    }
    return 0xFF;
}

void PPU::write(Word address, Byte value) {
    if (address >= 0x8000 && address <= 0x9FFF) { vram[address - 0x8000] = value; return; }
    if (address >= 0xFE00 && address <= 0xFE9F) { oam[address - 0xFE00] = value; return; }
    
    switch (address) {
        case 0xFF40: lcdc = value; break;
        case 0xFF41: stat = (value & 0xF8) | (stat & 0x07); break; // Los bits bajos de STAT son Read-Only
        case 0xFF42: scy = value; break;
        case 0xFF43: scx = value; break;
        case 0xFF44: break; // LY es Read-Only, no se puede escribir
        case 0xFF45: lyc = value; break;
        case 0xFF46: dma = value; break; // TODO: Implementar transferencia DMA
        case 0xFF47: bgp = value; break;
        case 0xFF48: obp0 = value; break;
        case 0xFF49: obp1 = value; break;
        case 0xFF4A: wy = value; break;
        case 0xFF4B: wx = value; break;
    }
}