#pragma once
#include <vector>
#include "../types.h"

class MBC {
protected:
    const std::vector<Byte>& rom;
    std::vector<Byte>& externalRAM;

public:
    MBC(const std::vector<Byte>& romData, std::vector<Byte>& ramData): rom(romData), externalRAM(ramData) {}
    virtual ~MBC() = default;

    virtual Byte read(Word address) const = 0;
    virtual void write(Word address, Byte value) = 0;
};
