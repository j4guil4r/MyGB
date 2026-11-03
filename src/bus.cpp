#include "bus.h"

Bus::Bus() {
    // Inicializamos las memorias internas a 0 para no tener basura
    // Nota: cartridgeMemory se inicializa vacía y crece al cargar la ROM
    vram.fill(0);
    wram.fill(0);
    oam.fill(0);
    hram.fill(0);
}


Byte Bus::read(Word addr) const {
    if (addr == 0xFF01) {
        return serialData;
    }
    if (addr == 0xFF02) {
        return 0x00;
    }
    // 1. ROM (0000 - 7FFF)
    if (addr < 0x8000) {
        return cartridge.read(addr);
    }
    // 2. VRAM (8000 - 9FFF)
    else if (addr >= 0x8000 && addr <= 0x9FFF) {
        return ppu.read(addr); 
    }
    // 3. External RAM (A000 - BFFF)
    else if (addr >= 0xA000 && addr <= 0xBFFF) {
        return cartridge.read(addr);
    }
    // 4. WRAM (C000 - DFFF)
    else if (addr >= 0xC000 && addr <= 0xDFFF) {
        return wram[addr - 0xC000];
    }
    // 5. ECHO RAM (E000 - FDFF)
    else if (addr >= 0xE000 && addr <= 0xFDFF) {
        return 0;
    }
    // 6. OAM (FE00 - FE9F)
    else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        return ppu.read(addr);
    }
    // 7. Unusable (FEA0 - FEFF)
    else if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return 0;
    }

    // 8. IO REGISTERS (FF00 - FF7F)
    else if (addr >= 0xFF00 && addr <= 0xFF7F) {

        // --- JOYPAD ---
        if (addr == 0xFF00) {
            Byte result = 0xCF; // Los bits 6 y 7 siempre devuelven 1 en hardware real
            result &= joypadSelect; // Mantenemos los bits de selección que el juego escribió

            // Si el Bit 4 es 0, el juego quiere leer Direcciones (Flechas)
            if ((joypadSelect & 0x10) == 0) {
                result &= joypadDir;
            }
            // Si el Bit 5 es 0, el juego quiere leer Acción (A, B, Select, Start)
            if ((joypadSelect & 0x20) == 0) {
                result &= joypadAction;
            }
            return result;
        }

        // --- APU ---
        if (addr >= 0XFF10 && addr <= 0xFF3F) {
            return apu.read(addr);
        }

        // --- Registros de la PPU (FF40 - FF4B) ---
        if (addr >= 0xFF40 && addr <= 0xFF4B) {
            return ppu.read(addr);
        }

        // Interrupciones
        if (addr == 0xFF0F) return ifRegister;

        // Timers
        if (addr == 0xFF04) {
            return (internalDiv >> 8) & 0xFF;
        }
        if (addr == 0xFF05) {return tima;}
        if (addr == 0xFF06) return tma;
        if (addr == 0xFF07) return tac;

        return 0;
    }

    // 9. HRAM (FF80 - FFFE)
    else if (addr >= 0xFF80 && addr <= 0xFFFE) {
        return hram[addr - 0xFF80];
    }
    // 10. Interrupt Enable (FFFF)
    else if (addr == 0xFFFF) {
        return ieRegister;
    }

    return 0xFF;
}

void Bus::write(Word addr, Byte data) {
    // --- PUERTO SERIAL ---
    if (addr == 0xFF01) {
        serialData = data;
        auto c = static_cast<char>(data);
        serialOutput += c;
        return;
    }
    else if (addr == 0xFF02) {
        // Interceptor de Blargg
        if (data == 0x81) {
            std::cout << (char)serialData << std::flush;
            data &= 0x7F;
        }
        return;
    }

    // 1. ROM
    if (addr < 0x8000) {
        cartridge.write(addr, data);
        return;
    }
    // 2. VRAM
    else if (addr >= 0x8000 && addr <= 0x9FFF) {
        ppu.write(addr, data); 
        return;
    }

    else if (addr >= 0xA000 && addr <= 0xBFFF) {
        cartridge.write(addr, data);
        return;
    }
    
    // 3. WRAM
    else if (addr >= 0xC000 && addr <= 0xDFFF) {
        wram[addr - 0xC000] = data;
        return;
    }
    // 4. OAM
    else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        ppu.write(addr, data);
        return;
    }

    // 5. IO REGISTERS (FF00 - FF7F)
    else if (addr >= 0xFF00 && addr <= 0xFF7F) {

        // --- DMA TRANSFER (0xFF46) ---
        if (addr == 0xFF46) {
            // El juego nos da el byte alto de la dirección.
            // Si data es 0xC1, la dirección fuente es 0xC100
            Word sourceAddress = data << 8; 
            
            for (int i = 0; i < 160; i++) {
                // Usamos nuestro propio Bus::read para sacar el dato, 
                // y lo metemos directo en la OAM de la PPU
                ppu.write(0xFE00 + i, this->read(sourceAddress + i));
            }
            return;
        }

        // --- APU ---
        if (addr >= 0xFF10 && addr <= 0xFF3F) {
            apu.write(addr, data);
            return;
        }

        // --- Registros de la PPU (FF40 - FF4B) ---
        if (addr >= 0xFF40 && addr <= 0xFF4B) {
            ppu.write(addr, data);
            return;
        }

        // --- JOYPAD ---
        if (addr == 0xFF00) {
            // El juego SOLO puede escribir en los bits 4 y 5 para seleccionar qué leer.
            // Protegemos el resto de los bits.
            joypadSelect = (data & 0x30) | 0xCF; 
            return;
        }

        // Timers
        else if (addr == 0xFF04) { 
            bool oldEdge = getTimerEdge(internalDiv, tac);
            internalDiv = 0; 
            bool newEdge = getTimerEdge(internalDiv, tac);
            
            if (oldEdge && !newEdge) {
                if (tima == 0xFF) { tima = tma; requestInterrupt(2); } 
                else { tima++; }
            }
            apu.resetSequencerPhase();
            return;
        }
        else if (addr == 0xFF05) {
            tima = data; return;}
        else if (addr == 0xFF06) {tma = data;return;}
        else if (addr == 0xFF07) {
            //tac = data;
            bool oldEdge = getTimerEdge(internalDiv, tac);
            tac = data; 
            bool newEdge = getTimerEdge(internalDiv, tac);
            
            if (oldEdge && !newEdge) {
                if (tima == 0xFF) { tima = tma; requestInterrupt(2); } 
                else { tima++; }
            }
            return;
        }

        // Interrupciones
        else if (addr == 0xFF0F) ifRegister = data;
    }

    // 6. HRAM
    else if (addr >= 0xFF80 && addr <= 0xFFFE) {
        hram[addr - 0xFF80] = data;
        return;
    }
    // 7. IE Register
    else if (addr == 0xFFFF) {
        ieRegister = data;
        return;
    }
}
void Bus::requestInterrupt(int bit) {
    ifRegister |= (1 << bit);
}

void Bus::updateTimers(const long long cycles) {
    if (timaOverflowDelay > 0) {
        timaOverflowDelay -= cycles;
        if (timaOverflowDelay <= 0) {
            tima = tma;
            requestInterrupt(2);
        }
    }

    bool oldEdge = getTimerEdge(internalDiv, tac);
    internalDiv += cycles;
    bool newEdge = getTimerEdge(internalDiv, tac);

    if (oldEdge && !newEdge) {
        if (tima == 0xFF) {
            tima = 0x00;
            timaOverflowDelay = 4;
        } else {
            tima++;
        }
    }
}

std::string Bus::getSerialOutput () const {
    return serialOutput;
}