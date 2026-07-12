#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "types.h"

class Cartridge {
private:
    std::vector<Byte> rom;
    std::vector<Byte> externalRAM;

    // Metadatos del header
    std::string title;
    Byte cartridgeType; // chip mbc
    Byte romSize; // peso | paginas de 16KB
    Byte ramSize; // si existe, tamaño de ram

    // --- Estado del MBC1 ---
    Byte currentROMBank = 1;
    Byte currentRAMBank = 0;
    bool ramEnabled = false;
    bool bankingMode = false;

    void parseHeader();

    // --- Persistencia ---
    std::string saveFilepath;
    bool hasBattery = false;
    void loadBattery();

public:
    Cartridge();
    ~Cartridge();

    bool loadROM(const std::string& filepath);
    void saveBattery();
    Byte read(Word address) const;
    void write(Word address, Byte value);

    // getters
    std::string getTitle() const {return title;}
    Byte getType() const {return cartridgeType;}
};