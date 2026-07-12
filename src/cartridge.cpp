#include "cartridge.h"
#include <fstream>
#include <iostream>

Cartridge::Cartridge() {
    cartridgeType = 0;
    romSize = 0;
    ramSize = 0;
}

Cartridge::~Cartridge(){
    saveBattery();
}

void Cartridge::loadBattery(){
    if(!hasBattery || externalRAM.empty()) return;

    std::ifstream file(saveFilepath, std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(externalRAM.data()), externalRAM.size());
        std::cout << "-> Bateria cargada exitosamente: " << saveFilepath << "\n";
    }
    else {
        std::cout << "-> No se encontro archivo de guardado previo. Se iniciara una partida nueva.\n";
    }
}

void Cartridge::saveBattery() {
    if (!hasBattery || externalRAM.empty()) return;

    std::ofstream file(saveFilepath, std::ios::binary | std::ios::trunc);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(externalRAM.data()), externalRAM.size());
        std::cout << "-> Partida guardada en: " << saveFilepath << "\n";
    }
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

    size_t dotPos = filepath.find_last_of('.');
    if (dotPos != std::string::npos) saveFilepath = filepath.substr(0, dotPos) + ".sav";
    else saveFilepath = filepath + ".sav";

    if(hasBattery) loadBattery();

    return true;
}

void Cartridge::parseHeader() {
    title = "";
    for(Word i = 0x0134; i <= 0x0143; ++i) {
        if(rom[i] == 0) break;
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

    hasBattery = (cartridgeType == 0x03 || cartridgeType == 0x0F || cartridgeType == 0x10 || cartridgeType == 0x13);

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
        if (!ramEnabled) return 0xFF;

        if (currentRAMBank >= 0x08) return 0x00;

        if (externalRAM.empty()) return 0xFF;
        
        Word offset = address - 0xA000;
        uint32_t targetAddress = (currentRAMBank * 0x2000) + offset;
        
        if (targetAddress < externalRAM.size()) return externalRAM[targetAddress];
        return 0xFF;
    }

    return 0xFF;
}

void Cartridge::write(Word address, Byte value) {
    bool isMBC1 = (cartridgeType >= 0x01 && cartridgeType <= 0x03);
    bool isMBC3 = (cartridgeType >= 0x0F && cartridgeType <= 0x13);

    if (isMBC1 || isMBC3) {
        
        // Comando 1: Habilitar/Deshabilitar RAM y Reloj (0x0000 - 0x1FFF)
        if (address <= 0x1FFF) {
            ramEnabled = ((value & 0x0F) == 0x0A);
        }
        
        // Comando 2: Cambiar de Banco ROM (0x2000 - 0x3FFF)
        else if (address >= 0x2000 && address <= 0x3FFF) {
            if (isMBC1) {
                Byte lower5 = value & 0x1F;
                if (lower5 == 0) lower5 = 1;
                currentROMBank = (currentROMBank & 0xE0) | lower5;
            } else if (isMBC3) {
                // MBC3 usa 7 bits directos y no tiene el bug de 0x20/0x40/0x60
                currentROMBank = value & 0x7F;
                if (currentROMBank == 0) currentROMBank = 1;
            }
        }
        
        // Comando 3: Cambiar Banco RAM o Registro del Reloj (0x4000 - 0x5FFF)
        else if (address >= 0x4000 && address <= 0x5FFF) {
            if (isMBC1) {
                if (bankingMode) {
                    currentRAMBank = value & 0x03;
                } else {
                    currentROMBank = (currentROMBank & 0x1F) | ((value & 0x03) << 5);
                    if (currentROMBank == 0) currentROMBank = 1;
                }
            } else if (isMBC3) {
                // En MBC3, 0x00-0x03 es RAM, 0x08-0x0C es Reloj
                currentRAMBank = value; 
            }
        }
        
        // Comando 4: Modo Banking o Latch de Reloj (0x6000 - 0x7FFF)
        else if (address >= 0x6000 && address <= 0x7FFF) {
            if (isMBC1) {
                bankingMode = (value & 0x01);
            } else if (isMBC3) {
                // Latch Clock del MBC3. Lo ignoramos en esta implementación rápida.
            }
        }
    }

    // Escritura normal en la RAM Externa (Guardar Partida)
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramEnabled) return;
        
        // Evitamos escribir en el vector de RAM si el juego está mandando datos al Reloj (MBC3)
        if (currentRAMBank >= 0x08) return;
        
        if (externalRAM.empty()) return;
        
        Word offset = address - 0xA000;
        uint32_t targetAddress = (currentRAMBank * 0x2000) + offset;
        
        if (targetAddress < externalRAM.size()) {
            externalRAM[targetAddress] = value;
        }
    }
}