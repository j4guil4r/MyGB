#include "Bus.h"

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
        // Protección simple para no leer fuera del vector
        if (addr < cartridgeMemory.size())
            return cartridgeMemory[addr];
        return 0;
    }

    // 2. VRAM (8000 - 9FFF)
    else if (addr >= 0x8000 && addr <= 0x9FFF) {
        return vram[addr - 0x8000];
    }

    // 3. External RAM (A000 - BFFF)
    else if (addr >= 0xA000 && addr <= 0xBFFF) {
        // TODO: Implementar RAM externa del cartucho
        return 0;
    }

    // 4. WRAM (C000 - DFFF)
    else if (addr >= 0xC000 && addr <= 0xDFFF) {
        return wram[addr - 0xC000];
    }

    // 5. ECHO RAM (E000 - FDFF)
    else if (addr >= 0xE000 && addr <= 0xFDFF) {
        return 0; // Ignoramos
    }

    // 6. OAM (FE00 - FE9F)
    else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        return oam[addr - 0xFE00];
    }

    // 7. Unusable (FEA0 - FEFF)
    else if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return 0;
    }

    // 8. IO Registers (FF00 - FF7F)
    else if (addr >= 0xFF00 && addr <= 0xFF7F) {
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
        vram[addr - 0x8000] = data;
    }

    // 3. WRAM
    else if (addr >= 0xC000 && addr <= 0xDFFF) {
        wram[addr - 0xC000] = data;
    }

    // 4. OAM
    else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        oam[addr - 0xFE00] = data;
    }

    // 5. IO Registers
    else if (addr >= 0xFF00 && addr <= 0xFF7F) {
        // --- TEST DE SALIDA SERIAL (Blargg) ---
        // Este es el puerto mágico para debug
        if (addr == 0xFF01) {
            std::cout << (char)data;
        }
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