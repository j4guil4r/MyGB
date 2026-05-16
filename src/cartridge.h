#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "types.h"

class Cartridge {
private:
    std::vector<Byte> rom;

    // Metadatos del header
    std::string title;
    Byte cartridgeType; // chip mbc
    Byte romSize; // peso | paginas de 16KB
    Byte ramSize; // si existe, tamaño de ram

    void parseHeader();

public:
    Cartridge();
    ~Cartridge() = default;

    bool loadROM(const std::string& filepath);
    Byte read(Word address) const;
    void write(Word address, Byte value);

    // getters
    std::string getTitle() const {return title;}
    Byte getType() const {return cartridgeType;}
};