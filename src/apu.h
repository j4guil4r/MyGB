#pragma once
#include "types.h"
#include <vector>
#include <SDL2/SDL.h>

class APU{
private:
    std::vector<float> audioBuffer;
    bool soundEnabled;

public:
    APU();
    ~APU();

    Byte read(Word address) const;
    void write(Word address, Byte value);

    void step(int cycles);
};