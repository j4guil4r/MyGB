#include "mbc1.h"

MBC1::MBC1(const std::vector<Byte>& romData, std::vector<Byte>& ramData): 
    MBC(romData, ramData), ramEnabled(false), bankingMode(false), currentROMBank(1), currentRAMBank(0) {}

Byte MBC1::read(Word address) const {
    // Calculamos los bancos físicos reales del archivo (Ej. 64KB = 4 bancos)
    size_t totalRomBanks = rom.size() / 0x4000;
    if (totalRomBanks == 0) totalRomBanks = 1; // Seguridad paranoica

    // 1. ROM Bank 00 (Fijo/Espejo - 0x0000 a 0x3FFF)
    if (address <= 0x3FFF) {
        Byte zeroBank = 0;
        // En MBC1, si bankingMode es 1, el Banco 0 cambia usando la RAM
        if (bankingMode == 1) {
            zeroBank = (currentRAMBank << 5);
        }
        zeroBank = zeroBank % totalRomBanks; // Wrap-around por hardware
        return rom[(zeroBank * 0x4000) + address];
    }
    
    // 2. ROM Bank 01-7F (Intercambiable - 0x4000 a 0x7FFF)
    if (address >= 0x4000 && address <= 0x7FFF) {
        Word offset = address - 0x4000;
        
        // ¡LA MAGIA DEL HARDWARE! Forzamos el espejo en lugar de devolver 0xFF
        Byte romBankToUse = currentROMBank % totalRomBanks;
        
        uint32_t targetAddress = (romBankToUse * 0x4000) + offset;
        return rom[targetAddress];
    }

    // 3. RAM Externa (0xA000 a 0xBFFF)
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramEnabled || externalRAM.empty()) return 0xFF;
        
        Word offset = address - 0xA000;
        
        Byte ramBankToUse = bankingMode ? currentRAMBank : 0;
        uint32_t targetAddress = (ramBankToUse * 0x2000) + offset;
        
        if (targetAddress < externalRAM.size()) return externalRAM[targetAddress];
        return 0xFF; // Aquí sí es válido devolver 0xFF si excede la RAM
    }

    return 0xFF;
}

void MBC1::write(Word address, Byte value) {
    // Comando 1: Habilitar/Deshabilitar RAM (0x0000 - 0x1FFF)
    if (address <= 0x1FFF) {
        ramEnabled = ((value & 0x0F) == 0x0A);
    }
    
    // Comando 2: Cambiar de Banco ROM (0x2000 - 0x3FFF)
    else if (address >= 0x2000 && address <= 0x3FFF) {
        Byte lower5 = value & 0x1F;
        if (lower5 == 0) lower5 = 1; // Evita el banco 0
        currentROMBank = (currentROMBank & 0xE0) | lower5;
    }
    
    // Comando 3: Cambiar Banco RAM o Bits altos del Banco ROM (0x4000 - 0x5FFF)
    else if (address >= 0x4000 && address <= 0x5FFF) {
        currentRAMBank = value & 0x03;
        // Se actualizan los bits 5 y 6 del currentROMBank
        currentROMBank = (currentROMBank & 0x1F) | (currentRAMBank << 5);
    }
    
    // Comando 4: Modo Banking (0x6000 - 0x7FFF)
    else if (address >= 0x6000 && address <= 0x7FFF) {
        bankingMode = (value & 0x01);
    }

    // Escritura normal en la RAM Externa (0xA000 a 0xBFFF)
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramEnabled || externalRAM.empty()) return;
        
        Word offset = address - 0xA000;
        
        Byte ramBankToUse = bankingMode ? currentRAMBank : 0;
        uint32_t targetAddress = (ramBankToUse * 0x2000) + offset;
        
        if (targetAddress < externalRAM.size()) {
            externalRAM[targetAddress] = value;
        }
    }
}