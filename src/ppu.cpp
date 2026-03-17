#include "ppu.h"

constexpr uint32_t GB_COLOR_0 = 0xFFFFFFFF;  // Blanco
constexpr uint32_t GB_COLOR_1 = 0xFFAAAAAA;  // Gris claro
constexpr uint32_t GB_COLOR_2 = 0xFF555555;  // Gris oscuro
constexpr uint32_t GB_COLOR_3 = 0xFF000000;  // Negro

constexpr uint32_t GB_PALETTE[4] = {
    GB_COLOR_0,
    GB_COLOR_1,
    GB_COLOR_2,
    GB_COLOR_3
};


PPU::PPU() {
    reset();
}


PPU::~PPU(){}

inline int PPU::decode2bpp(Byte lo, Byte hi, int bit) {
    return ((hi >> bit) & 1) << 1 | ((lo >> bit) & 1);
}

inline Byte PPU::readVRAM(Word addr){
    return vram[addr - 0x8000];
}

// Devuelve el color ARGB final de un píxel específico dentro de un Tile
uint32_t PPU::getTilePixelColor(Byte tileNumber, Word tileDataBase, Byte line, Byte xPixel, Byte paletteReg) {
    // Calcular ubicación real del Tile
    Word tileLocation = tileDataBase;
    if (tileDataBase == 0x8000) {
        tileLocation += (tileNumber * 16);
    } else {
        tileLocation = 0x9000 + (static_cast<int8_t>(tileNumber) * 16);
    }

    // Leer los 2 bytes de la línea (¡Usando tu nuevo readVRAM!)
    Byte data1 = readVRAM(tileLocation + (line * 2));
    Byte data2 = readVRAM(tileLocation + (line * 2) + 1);

    // Decodificar
    int colorBit = 7 - (xPixel % 8);
    int colorNum = decode2bpp(data1, data2, colorBit);

    // Aplicar paleta y retornar color final
    int paletteColor = (paletteReg >> (colorNum * 2)) & 0x03;
    return GB_PALETTE[paletteColor];
}

void PPU::reset() {
    framebuffer.fill(GB_COLOR_0); 
    frameReady = false;
}

// Helper para cambiar el modo sin destruir los otros bits importantes del STAT
void PPU::setMode(int mode) {
    // Apagamos los 2 bits más bajos (modo actual) y le hacemos OR con el nuevo modo
    stat = (stat & ~0x03) | (mode & 0x03);
}

void PPU::drawSprites() {
    // El bit 1 del LCDC (0xFF40) controla si los sprites son visibles.
    if ((lcdc & 0x02) == 0) return;

    // Tamaño: El bit 2 del LCDC dice si son de 8x8 (0) o de 8x16 (1) píxeles.
    int spriteHeight = (lcdc & 0x04) ? 16 : 8;

    for (int sprite = 0; sprite < 40; sprite++) {
        // Obtenemos el índice base de este sprite en el arreglo OAM
        int index = sprite * 4;

        int yPos = oam[index] - 16;
        int xPos = oam[index + 1] - 8;
        Byte tileIndex = oam[index + 2];
        Byte attributes = oam[index + 3];

        // ¿Este Sprite cruza la línea (LY) que estamos dibujando ahora mismo?
        if (ly >= yPos && ly < (yPos + spriteHeight)) {
            
            // Extraemos las banderas de los Atributos
            bool flipY = (attributes & 0x40) != 0;
            bool flipX = (attributes & 0x20) != 0;
            bool objBehindBg = (attributes & 0x80) != 0;
            
            // ¿Qué paleta usa? (OBP0 u OBP1)
            Byte palette = (attributes & 0x10) ? obp1 : obp0;

            // Calcular qué fila del Sprite (de la 0 a la 7 o 15) estamos dibujando
            int line = ly - yPos;

            // Si el sprite está volteado de cabeza (Flip Y), leemos la línea desde abajo
            if (flipY) {
                line = (spriteHeight - 1) - line;
            }

            // Si es un sprite de 8x16, el hardware ignora el bit más bajo del tileIndex
            if (spriteHeight == 16) {
                tileIndex &= 0xFE; // Forzamos a que sea un número par
            }

            // Buscar los 2 bytes que forman la línea en la VRAM (0x8000)
            Word tileLocation = 0x8000 + (tileIndex * 16);
            Byte data1 = vram[(tileLocation + (line * 2)) - 0x8000];
            Byte data2 = vram[(tileLocation + (line * 2) + 1) - 0x8000];

            // Dibujar los 8 píxeles horizontales de este Sprite
            for (int tilePixel = 7; tilePixel >= 0; tilePixel--) {
                
                // Calculamos en qué coordenada X de la pantalla cae este píxel
                int colorBit = tilePixel;
                
                // Si el sprite está espejado horizontalmente (Flip X), leemos los bits al revés
                if (flipX) {
                    colorBit = 7 - colorBit;
                }

                int pixelX = xPos + (7 - tilePixel);

                // Evitamos dibujar píxeles que caen fuera de la pantalla
                if (pixelX < 0 || pixelX >= 160) continue;

                // Extraemos el color ID (igual que con el fondo)
                int colorNum = decode2bpp(data1, data2, colorBit);

                // El Color 0 es transparente en los Sprites
                if (colorNum == 0) continue;

                // Determinar el color final usando la paleta seleccionada
                int paletteColor = (palette >> (colorNum * 2)) & 0x03;
                
                uint32_t finalColor = GB_PALETTE[paletteColor];

                // Prioridad Z: ¿El sprite va detrás del fondo?
                // Si objBehindBg es true, el sprite SOLO se dibuja si el píxel del fondo era color 0 (blanco)
                if (objBehindBg) {
                    uint32_t bgColor = framebuffer[(ly * GB_WIDTH) + pixelX];
                    if (bgColor != GB_COLOR_0) continue;
                }

                // Finalmente, plasmamos el píxel en el lienzo
                framebuffer[(ly * GB_WIDTH) + pixelX] = finalColor;
            }
        }
    }
}

void PPU::drawScanline() {
    // El bit 0 del registro LCDC (0xFF40) controla si el fondo se dibuja o no.
    if ((lcdc & 0x01) == 0) {
        // Si está apagado, la Game Boy original dibuja la línea de color blanco (o gris clarito)
        for (int x = 0; x < GB_WIDTH; x++) {
            framebuffer[(ly * GB_WIDTH) + x] = GB_COLOR_0; // Blanco ARGB
        }
        return;
    }

    // 2. ¿Dónde está el Mapa de Tiles (Tile Map) en la memoria?
    // El bit 3 del LCDC nos dice si el mapa base está en 0x9800 o en 0x9C00
    Word tileMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;

    // 3. ¿Dónde están los dibujos de los Tiles (Tile Data) en la memoria?
    // El bit 4 del LCDC nos dice de dónde sacar los píxeles crudos (0x8000 o 0x8800)
    Word tileDataBase = (lcdc & 0x10) ? 0x8000 : 0x8800;

    // 4. Calcular la posición real en el mapa usando el Scroll
    // SCY (0xFF42) es el desplazamiento vertical de la cámara.
    // Sumamos la línea actual de la pantalla (LY) con el Scroll para saber qué fila del mapa mirar.
    Byte yPos = ly + scy;

    // El mapa es una cuadrícula de 32x32 Tiles. Cada Tile tiene 8x8 píxeles.
    // Dividiendo yPos entre 8, sabemos en qué "fila de Tiles" estamos (de la 0 a la 31).
    Word tileRow = (yPos / 8) * 32;

    // 5. Dibujar los 160 píxeles horizontales de esta línea (ly)
    for (int x = 0; x < GB_WIDTH; x++) {
        Byte xPos = x + scx;
        Word tileAddress = tileMapBase + tileRow + (xPos >> 3);
        
        Byte tileNumber = readVRAM(tileAddress);
        Byte line = yPos % 8;

        framebuffer[(ly * GB_WIDTH) + x] = getTilePixelColor(tileNumber, tileDataBase, line, xPos, bgp);
    }
}

void PPU::drawWindow() {
    // 1. ¿Está encendida la Ventana?
    // El bit 5 del LCDC (0xFF40) controla si la ventana es visible.
    if ((lcdc & 0x20) == 0) return;

    // 2. ¿La línea que estamos dibujando ahora mismo (LY) cruzó el borde de la ventana (WY)?
    if (ly < wy) return; // Si la ventana empieza más abajo, no hacemos nada todavía

    // 3. ¿Dónde está el Mapa de la Ventana?
    // El bit 6 del LCDC nos dice si el mapa base está en 0x9800 o 0x9C00
    Word tileMapBase = (lcdc & 0x40) ? 0x9C00 : 0x9800;

    // 4. ¿Dónde están los gráficos? (Igual que el fondo, bit 4)
    Word tileDataBase = (lcdc & 0x10) ? 0x8000 : 0x8800;

    // La posición Y interna de la ventana (desde su propio "borde superior")
    Byte windowY = ly - wy;
    
    // Calculamos qué fila de Tiles del mapa nos toca leer
    Word tileRow = (windowY / 8) * 32;

    int realWx = wx - 7;

    // 5. Dibujar los 160 píxeles de la línea
    for (int x = 0; x < GB_WIDTH; x++) {
        if (x < realWx) continue;

        Byte windowX = x - realWx;
        Word tileAddress = tileMapBase + tileRow + (windowX >> 3);
        
        Byte tileNumber = readVRAM(tileAddress);
        Byte line = windowY % 8;

        framebuffer[(ly * GB_WIDTH) + x] = getTilePixelColor(tileNumber, tileDataBase, line, windowX, bgp);
    }
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
                drawScanline(); 
                drawWindow();
                drawSprites();
            }
            break;

        case PPUMode::HBlank: // Modo 0
            if (dots >= 204) {
                dots -= 204;
                ly++;

                if (ly == 144) {
                    currentMode = PPUMode::VBlank;
                    frameReady = true;
                    requestVBlankInterrupt = true;
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
    setMode(static_cast<Byte> (currentMode));

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