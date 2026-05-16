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
    memblock = reinterpret_cast<char*> rom.data(); 
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

    std::cout << "--- CARTUCHO CARGADO ---\n";
    std::cout << "Titulo: " << title << "\n";
    std::cout << "Tipo (Hex): 0x" << std::hex << (int)cartridgeType << std::dec << "\n";
    std::cout << "------------------------\n";
}

Byte Cartridge::read(Word address) const {
    // Por ahora, nos comportamos como un juego simple de 32KB (ROM Only)
    // TODO: switch(cartridgeType) para el MBC1
    if (address < rom.size()) {
        return rom[address];
    }
    return 0xFF;
}

void Cartridge::write(Word address, Byte value) {}