#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "types.h"

class Bus {
public:
    Bus(){memory.fill(0);}
    ~Bus() = default;

    Byte read(Word addr) const {
        // TODO: redirigir al cartucho, vram, ...
        if (addr < memory.size()) {
            return memory[addr];
        }
        return 0;
    }

    void write(Word addr, Byte data) {
        if (addr < memory.size())
            memory[addr] = data;
    }

    bool loadROM(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            std::cerr << "Error: No se pudo abrir el archivo " << filename << "\n";
            return false;
        }

        std::streampos size = file.tellg();
        // Apuntar al inicio del archivo
        file.seekg(0, std::ios::beg);

        std::cout << "Cargando ROM: " << filename << " | Size: " << size << " bytes\n";

        // Validaciones básicas (La Boot ROM debe ser de 256 bytes)
        if (size > MEMORY_SIZE) {
            std::cerr << "Error: ROM demasiado grande para la memoria base.\n";
            return false;
        }

        file.read(reinterpret_cast<char*>(memory.data()), size);

        file.close();
        return true;
    }

    std::array<Byte, MEMORY_SIZE> memory;
};