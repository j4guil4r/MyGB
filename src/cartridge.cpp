#include "cartridge.h"
#include <fstream>
#include <iostream>

Cartridge::Cartridge() {
    cartridgeType = 0;
    romSize = 0;
    ramSize = 0;
}

bool Cartridge::loadROM(const std::string& filepath){
    char* memblock = nullptr;
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if(!file.is_open()) return false;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    rom.resize(size);
    memblock = reinterpret_cast<char*>(rom.data()); 
    if(!file.read(memblock, size)) {
        return false;
    }

    parseHeader();
    return true;
}

void Cartridge::parseHeader() {
    title = "";
    for(Word i = 0x0134; i <= 0x0143; ++i) {
        if(rom[0] == 0) break;
        title += static_cast<char>(rom[i]);
    }

    cartridgeType = rom[0x0147];
    romSize = rom[0x0148];
    ramSize = rom[0x0149];

    // Preparar la RAM Externa dependiendo de ramSize (0x0149)
    switch (ramSize) {
        case 0x02: externalRAM.resize(8 * 1024, 0); break;
        case 0x03: externalRAM.resize(32 * 1024, 0); break;
        case 0x04: externalRAM.resize(128 * 1024, 0); break;
        case 0x05: externalRAM.resize(64 * 1024, 0); break;
        default: externalRAM.clear(); break;
    }

    currentROMBank = 1;
    currentRAMBank = 0;
    ramEnabled = false;
    bankingMode = false;

    std::cout << "--- CARTUCHO CARGADO ---\n";
    std::cout << "Titulo: " << title << "\n";
    std::cout << "Tipo (Hex): 0x" << std::hex << (int)cartridgeType << std::dec << "\n";
    std::cout << "------------------------\n";
}

Byte Cartridge::read(Word address) const {
    // 1. ROM Bank 00 (Fijo - 0x0000 a 0x3FFF)
    if (address <= 0x3FFF) {
        return rom[address];
    }
    
    // 2. ROM Bank 01-NN (Intercambiable - 0x4000 a 0x7FFF)
    if (address >= 0x4000 && address <= 0x7FFF) {
        Word offset = address - 0x4000;
        // La dirección real en nuestro vector std::vector<Byte> rom:
        uint32_t targetAddress = (currentROMBank * 0x4000) + offset;
        
        if (targetAddress < rom.size()) return rom[targetAddress];
        return 0xFF;
    }

    // 3. RAM Externa (0xA000 a 0xBFFF)
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramEnabled || externalRAM.empty()) return 0xFF;
        
        Word offset = address - 0xA000;
        uint32_t targetAddress = (currentRAMBank * 0x2000) + offset;
        
        if (targetAddress < externalRAM.size()) return externalRAM[targetAddress];
        return 0xFF;
    }

    return 0xFF;
}

void Cartridge::write(Word address, Byte value) {
    // Solo manejamos MBC1 por ahora (Tipos 1, 2 y 3)
    if (cartridgeType >= 0x01 && cartridgeType <= 0x03) {
        
        // Comando 1: Habilitar/Deshabilitar RAM (0x0000 - 0x1FFF)
        if (address <= 0x1FFF) {
            // Se habilita escribiendo exactamente 0x0A en los 4 bits bajos
            ramEnabled = ((value & 0x0F) == 0x0A);
        }
        
        // Comando 2: Cambiar de Banco ROM (0x2000 - 0x3FFF)
        else if (address >= 0x2000 && address <= 0x3FFF) {
            // Tomamos los 5 bits bajos del valor.
            // OJO: currentROMBank podría tener configurados sus bits altos, no los borramos.
            currentROMBank = (currentROMBank & 0xE0) | (value & 0x1F);
            
            // ¡Trampa de hardware! El banco 0 se traduce como 1.
            if (currentROMBank == 0) currentROMBank = 1;
        }
        
        // Comando 3: Cambiar Banco RAM o Bits altos del Banco ROM (0x4000 - 0x5FFF)
        else if (address >= 0x4000 && address <= 0x5FFF) {
            if (bankingMode) {
                // Si estamos en modo RAM, cambiamos el currentRAMBank
                currentRAMBank = value & 0x03;
            } else {
                // Si estamos en modo ROM, estos 2 bits forman parte de currentROMBank (bits 5 y 6)
                currentROMBank = (currentROMBank & 0x1F) | ((value & 0x03) << 5);
                if (currentROMBank == 0) currentROMBank = 1;
            }
        }
        
        // Comando 4: Modo Banking (0x6000 - 0x7FFF)
        else if (address >= 0x6000 && address <= 0x7FFF) {
            bankingMode = (value & 0x01); // 0 = ROM, 1 = RAM
        }
    }

    // Escritura normal en la RAM Externa (Guardar Partida)
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramEnabled || externalRAM.empty()) return;
        
        Word offset = address - 0xA000;
        uint32_t targetAddress = (currentRAMBank * 0x2000) + offset;
        
        if (targetAddress < externalRAM.size()) {
            externalRAM[targetAddress] = value;
        }
    }
}