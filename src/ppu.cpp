#include "ppu.h"

constexpr uint32_t GB_COLOR_0 = 0xFFFFFFFF;  // Blanco
constexpr uint32_t GB_COLOR_1 = 0xFFAAAAAA;  // Gris claro
constexpr uint32_t GB_COLOR_2 = 0xFF555555;  // Gris oscuro
constexpr uint32_t GB_COLOR_3 = 0xFF000000;  // Negro

PPU::PPU() {
    reset();
}


PPU::~PPU(){}


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
                int colorBit0 = (data1 >> colorBit) & 0x01;
                int colorBit1 = (data2 >> colorBit) & 0x01;
                int colorNum = (colorBit1 << 1) | colorBit0;

                // El Color 0 es transparente en los Sprites
                if (colorNum == 0) continue;

                // Determinar el color final usando la paleta seleccionada
                int paletteColor = (palette >> (colorNum * 2)) & 0x03;
                
                uint32_t finalColor;
                switch (paletteColor) {
                    case 0: finalColor = GB_COLOR_0; // Blanco
                    case 1: finalColor = GB_COLOR_1; // Gris Claro
                    case 2: finalColor = GB_COLOR_2; // Gris Oscuro
                    case 3: finalColor = GB_COLOR_3; // Negro
                }

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
        
        // Calcular la posición X real sumando el Scroll Horizontal (SCX)
        Byte xPos = x + scx;

        // Dividimos entre 8 para saber en qué "columna de Tiles" estamos (de la 0 a la 31)
        Word tileCol = xPos / 8;

        // --- A. BUSCAR EL NÚMERO DEL TILE EN EL MAPA ---
        Word tileAddress = tileMapBase + tileRow + tileCol;
        
        // ¡OJO! La VRAM en nuestra clase PPU empieza en 0x8000, así que restamos ese offset
        // para leer nuestro arreglo `vram` interno.
        Byte tileNumber = vram[tileAddress - 0x8000];

        // --- B. BUSCAR LOS PÍXELES DEL TILE (Decodificación 2bpp) ---
        Word tileLocation = tileDataBase;
        
        if ((lcdc & 0x10) != 0) {
            // Si usamos la base 0x8000, los números de Tile son sin signo (0 a 255)
            // Cada Tile ocupa 16 bytes.
            tileLocation += (tileNumber * 16);
        } else {
            // Si usamos la base 0x8800, los números son CON signo (-128 a 127)
            // y la base real empieza en 0x9000.
            auto signedTileNum = static_cast<int8_t>(tileNumber);
            tileLocation = 0x9000 + (signedTileNum * 16);
        }

        // ¿Qué línea específica de las 8 que tiene el Tile estamos dibujando? (0 a 7)
        Byte line = yPos % 8;

        // Leemos los 2 bytes que forman la línea de este Tile (cada línea ocupa 2 bytes)
        Byte data1 = vram[(tileLocation + (line * 2)) - 0x8000];     // Byte bajo
        Byte data2 = vram[(tileLocation + (line * 2) + 1) - 0x8000]; // Byte alto

        // --- C. OBTENER EL COLOR DEL PÍXEL (El algoritmo que no entendías) ---
        // ¿Qué píxel horizontal del Tile estamos dibujando? (0 a 7, donde 0 es la izquierda)
        // Como el bit 7 (el de más a la izquierda) corresponde al píxel 0, invertimos el índice:
        int colorBit = 7 - (xPos % 8);

        // Extraemos el bit correspondiente del data1 y data2, y los unimos
        int colorBit0 = (data1 >> colorBit) & 0x01;
        int colorBit1 = (data2 >> colorBit) & 0x01;
        
        int colorNum = (colorBit1 << 1) | colorBit0; // El resultado es 0, 1, 2 o 3

        // --- D. APLICAR LA PALETA (BGP - 0xFF47) ---
        // La Game Boy permite a los juegos cambiar cómo se ven los colores 0, 1, 2 y 3.
        // El registro BGP contiene la paleta actual.
        // Extraemos los 2 bits correspondientes de la paleta:
        int paletteColor = (bgp >> (colorNum * 2)) & 0x03;

        // --- E. PINTAR EN SDL2 ---
        // Asignamos un color real en formato ARGB dependiendo del valor final (0 a 3)
        uint32_t finalColor;
        switch (paletteColor) {
            case 0: finalColor = GB_COLOR_0; break;
            case 1: finalColor = GB_COLOR_1; break;
            case 2: finalColor = GB_COLOR_2; break;
            case 3: finalColor = GB_COLOR_3; break;
        }

        // Guardamos el píxel en nuestro lienzo de SDL2
        framebuffer[(ly * GB_WIDTH) + x] = finalColor;
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