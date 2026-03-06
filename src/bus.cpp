#include "bus.h"

Bus::Bus() {
    // Inicializamos las memorias internas a 0 para no tener basura
    // Nota: cartridgeMemory se inicializa vacía y crece al cargar la ROM
    vram.fill(0);
    wram.fill(0);
    oam.fill(0);
    hram.fill(0);

    // Reservamos un mínimo para evitar errores si leemos sin cargar ROM
    cartridgeMemory.resize(32 * 1024, 0);
}

// Carga del archivo .gb al vector de memoria
bool Bus::loadROM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << "\n";
        return false;
    }

    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "Cargando ROM: " << filename << " (" << size << " bytes)\n";

    cartridgeMemory.resize(size);
    file.read(reinterpret_cast<char*>(cartridgeMemory.data()), size);
    file.close();
    return true;
}

// EL MAPA DE MEMORIA (LECTURA)
Byte Bus::read(Word addr) const {
    // 1. ROM (0000 - 7FFF)
    if (addr < 0x8000) {
        if (addr < cartridgeMemory.size())
            return cartridgeMemory[addr];
        return 0;
    }
    // 2. VRAM (8000 - 9FFF)
    else if (addr >= 0x8000 && addr <= 0x9FFF) {
        return ppu.read(addr); 
    }
    // 3. External RAM (A000 - BFFF)
    else if (addr >= 0xA000 && addr <= 0xBFFF) {
        return 0; // TODO: RAM de Cartucho
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
        // --- Registros de la PPU (FF40 - FF4B) ---
        if (addr >= 0xFF40 && addr <= 0xFF4B) {
            return ppu.read(addr);
        }

        // Interrupciones
        if (addr == 0xFF0F) return ifRegister;

        // Timers
        if (addr == 0xFF04) return div;
        if (addr == 0xFF05) return tima;
        if (addr == 0xFF06) return tma;
        if (addr == 0xFF07) return tac;

        // Serial (Para debug, opcional en lectura)
        if (addr == 0xFF01) return 0;

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

    return 0;
}

// EL MAPA DE MEMORIA (ESCRITURA)
void Bus::write(Word addr, Byte data) {
    // 1. ROM
    if (addr < 0x8000) {
        return;
    }
    // 2. VRAM
    else if (addr >= 0x8000 && addr <= 0x9FFF) {
        ppu.write(addr, data); 
        return;
    }
    // 3. WRAM
    else if (addr >= 0xC000 && addr <= 0xDFFF) {
        wram[addr - 0xC000] = data;
    }
    // 4. OAM
    else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        ppu.write(addr, data);
        return;
    }

    // 5. IO REGISTERS (FF00 - FF7F)
    else if (addr >= 0xFF00 && addr <= 0xFF7F) {

        // --- Registros de la PPU (FF40 - FF4B) ---
        if (addr >= 0xFF40 && addr <= 0xFF4B) {
            ppu.write(addr, data);
            return;
        }

        // Serial Output (Debug Blargg)
        if (addr == 0xFF01) {
            auto c = static_cast<char>(data);
            serialOutput += c;
        }

        // Timers
        else if (addr == 0xFF04) { div = 0; divCounter = 0; }
        else if (addr == 0xFF05) tima = data;
        else if (addr == 0xFF06) tma = data;
        else if (addr == 0xFF07) tac = data;

        // Interrupciones
        else if (addr == 0xFF0F) ifRegister = data;
    }

    // 6. HRAM
    else if (addr >= 0xFF80 && addr <= 0xFFFE) {
        hram[addr - 0xFF80] = data;
    }
    // 7. IE Register
    else if (addr == 0xFFFF) {
        ieRegister = data;
    }
}
void Bus::requestInterrupt(int bit) {
    // Debug: Chivatear si es el Timer (Bit 2)
    /*if (bit == 2) {
        std::cout << "[INT] TIMER FIRED!\n";
    }*/
    // bit 0: VBlank, bit 1: LCD, bit 2: Timer, etc.
    ifRegister |= (1 << bit);
}

void Bus::updateTimers(const long long cycles) {
    // 1. DIV (Divider Register)
    // Incrementa siempre, a una velocidad de 16384 Hz.
    // La CPU va a 4194304 Hz. 4194304 / 16384 = 256 ciclos de CPU por cada tick de DIV.
    divCounter += cycles;
    if (divCounter >= 256) {
        div++;
        divCounter -= 256;
    }

    // 2. TIMA (Timer Counter)
    // Solo funciona si el bit 2 de TAC está encendido
    if ((tac & 0x04) != 0) {
        timerCounter += cycles;

        // Determinar frecuencia según TAC (bits 0-1)
        // 00: 4096 Hz (1024 ciclos)
        // 01: 262144 Hz (16 ciclos)
        // 10: 65536 Hz (64 ciclos)
        // 11: 16384 Hz (256 ciclos)
        int threshold = 0;
        switch (tac & 0x03) {
            case 0: threshold = 1024; break;
            case 1: threshold = 16; break;
            case 2: threshold = 64; break;
            case 3: threshold = 256; break;
        }

        while (timerCounter >= threshold) {
            timerCounter -= threshold;

            // Incrementamos TIMA
            if (tima == 0xFF) {
                // ¡OVERFLOW!
                tima = tma; // Recargamos con el valor de TMA
                requestInterrupt(2); // Pedimos Interrupción de Timer (Bit 2)
            } else {
                tima++;
            }
        }
    }

    // --- SIMULACIÓN PPU (Simple) ---
    ppuCounter += cycles;
    // Una línea tarda 456 ciclos de CPU
    if (ppuCounter >= 456) {
        ppuCounter -= 456;
        ly++;

        // Si llegamos a la línea 144, entramos en VBlank -> INT 0
        if (ly == 144) requestInterrupt(0);

        // Si pasamos de la línea 153, volvemos a empezar (Frame nuevo)
        else if (ly > 153) ly = 0;
    }
}

std::string Bus::getSerialOutput () const {
    return serialOutput;
}