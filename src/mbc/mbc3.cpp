#include "mbc3.h"

MBC3::MBC3(const std::vector<Byte>& romData, std::vector<Byte>& ramData):
    MBC(romData, ramData), ramAndTimerEnabled(false), currentROMBank(1), currentRAMBank(0) {}

Byte MBC3::read(Word address) const {
    // 1. ROM Bank 00 (Fijo - 0x0000 a 0x3FFF)
    if (address <= 0x3FFF) {
        return rom[address];
    }
    
    // 2. ROM Bank 01-7F (Intercambiable - 0x4000 a 0x7FFF)
    if (address >= 0x4000 && address <= 0x7FFF) {
        Word offset = address - 0x4000;
        uint32_t targetAddress = (currentROMBank * 0x4000) + offset;
        
        if (targetAddress < rom.size()) return rom[targetAddress];
        return 0xFF;
    }

    // 3. RAM Externa o Registros RTC (0xA000 a 0xBFFF)
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramAndTimerEnabled) return 0xFF;
        
        // Si es un registro del Reloj (RTC)
        if (currentRAMBank >= 0x08 && currentRAMBank <= 0x0C) {
            return 0x00; // Retornamos 0 temporalmente para no crashear
        }
        
        // Si es RAM normal
        if (currentRAMBank <= 0x03 && !externalRAM.empty()) {
            Word offset = address - 0xA000;
            uint32_t targetAddress = (currentRAMBank * 0x2000) + offset;
            
            if (targetAddress < externalRAM.size()) return externalRAM[targetAddress];
        }
        
        return 0xFF;
    }

    return 0xFF;
}

void MBC3::write(Word address, Byte value) {
    // Comando 1: Habilitar RAM y Timer (0x0000 - 0x1FFF)
    if (address <= 0x1FFF) {
        ramAndTimerEnabled = ((value & 0x0F) == 0x0A);
    }
    
    // Comando 2: Cambiar de Banco ROM (0x2000 - 0x3FFF)
    else if (address >= 0x2000 && address <= 0x3FFF) {
        // MBC3 usa 7 bits (0-127). El 0 se convierte en 1.
        currentROMBank = value & 0x7F;
        if (currentROMBank == 0) currentROMBank = 1;
    }
    
    // Comando 3: Cambiar Banco RAM o Registro RTC (0x4000 - 0x5FFF)
    else if (address >= 0x4000 && address <= 0x5FFF) {
        currentRAMBank = value; 
    }
    
    // Comando 4: Latch Clock Data (0x6000 - 0x7FFF)
    else if (address >= 0x6000 && address <= 0x7FFF) {
        // Ignoramos el reloj por ahora
    }

    // Escritura en RAM Externa o Registros RTC (0xA000 a 0xBFFF)
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramAndTimerEnabled) return;
        
        // Evitamos escribir en la RAM si estamos en modo Reloj
        if (currentRAMBank >= 0x08 && currentRAMBank <= 0x0C) {
            return; // Aquí iría la lógica del reloj en el futuro
        }
        
        if (currentRAMBank <= 0x03 && !externalRAM.empty()) {
            Word offset = address - 0xA000;
            uint32_t targetAddress = (currentRAMBank * 0x2000) + offset;
            
            if (targetAddress < externalRAM.size()) {
                externalRAM[targetAddress] = value;
            }
        }
    }
}