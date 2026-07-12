#pragma once
#include "mbc.h"

class MBC1: public MBC {
private:
    bool ramEnabled;
    // 0 = ROM, 1 = RAM
    bool bankingMode;
    Byte currentROMBank;
    Byte currentRAMBank;

public:
    MBC1(const std::vector<Byte>& romData, std::vector<Byte>& ramData);
    ~MBC1() override = default;
    
    Byte read(Word address) const override;
    void write(Word address, Byte value) override;
};