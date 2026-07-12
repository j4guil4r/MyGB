#include "cartridge.h"
#include "mbc/mbc1.h"
#include "mbc/mbc3.h"
#include <fstream>
#include <iostream>

Cartridge::Cartridge() {
    cartridgeType = 0;
    romSize = 0;
    ramSize = 0;
    mbc = nullptr;
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

    hasBattery = (cartridgeType == 0x03 || cartridgeType == 0x0F || cartridgeType == 0x10 || cartridgeType == 0x13);

    // --- INSTANCIAR EL CHIP MBC (Factory Pattern) ---
    if (cartridgeType >= 0x01 && cartridgeType <= 0x03) {
        mbc = std::make_unique<MBC1>(rom, externalRAM);
    } 
    else if (cartridgeType >= 0x0F && cartridgeType <= 0x13) {
        mbc = std::make_unique<MBC3>(rom, externalRAM);
    } 
    else {
        // ROM Only (ej. Tetris o Super Mario Land)
        mbc = nullptr;
    }

    std::cout << "--- CARTUCHO CARGADO ---\n";
    std::cout << "Titulo: " << title << "\n";
    std::cout << "Tipo (Hex): 0x" << std::hex << (int)cartridgeType << std::dec << "\n";
    std::cout << "------------------------\n";
}

Byte Cartridge::read(Word address) const {
    if(mbc) return mbc->read(address);

    // ROM only
    if(address <= 0x7FFF){
        if (address < rom.size()) return rom[address];
    }
    else if(address >= 0xA000 && address <= 0xBFFF){
        if(!externalRAM.empty()) {
            Word offset = address - 0xA000;
            if (offset < externalRAM.size()) return externalRAM[offset];
        }
    }
    return 0xFF;
}

void Cartridge::write(Word address, Byte value) {
    if(mbc){
        mbc->write(address, value);
        return;
    }

    // Comportamiento por defecto (ROM Only)
    // Los juegos ROM Only no pueden escribir en la ROM (0x0000-0x7FFF).
    // Si por alguna rareza tienen RAM externa (Tipo 0x08), escribimos ahí.
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (!externalRAM.empty()) {
            Word offset = address - 0xA000;
            if (offset < externalRAM.size()) externalRAM[offset] = value;
        }
    }
}