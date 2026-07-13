#pragma once
#include "mbc.h"

class MBC3 : public MBC {
private:
    bool ramAndTimerEnabled;
    Byte currentROMBank;
    // Puede ser un banco de RAM (00-03) o un registro de RTC (08-0C)
    Byte currentRAMBank;

public:
    MBC3(const std::vector<Byte>& romData, std::vector<Byte>& ramData);
    ~MBC3() override = default;

    Byte read(Word address) const override;
    void write(Word address, Byte value) override;
};