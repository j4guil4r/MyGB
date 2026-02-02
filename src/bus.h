#pragma once
#include <cstdint>
#include <array>
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

    std::array<Byte, MEMORY_SIZE> memory;
};