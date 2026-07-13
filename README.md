# MyGB - Game Boy Emulator in C++

A cycle-accurate Game Boy emulator written from scratch in modern C++. This project focuses on low-level system architecture, accurate hardware timing, and clean object-oriented design.

## Features
- **CPU:** Complete implementation of the Sharp LR35902 instruction set.
- **PPU:** Cycle-accurate pixel rendering (H-Blank, V-Blank, OAM, Transfer states).
- **Memory Management:** Unified Bus architecture.
- **Cartridges (MBC):** Support for ROM Only, MBC1, and MBC3 (includes external RAM battery saves for games like Pokémon).
- **Input:** SDL2-based Joypad interrupt mapping.

## Roadmap
- [x] CPU Instruction Set
- [x] PPU & Cycle Timing
- [x] MBC1 & MBC3 Support
- [x] Battery Saves (.sav)
- [ ] APU (Audio Processing Unit)
- [ ] WebAssembly / Browser Support via Emscripten

## Building the Project
### Requirements
* C++20 Compiler (GCC/Clang)
* CMake (3.28 or higher)
* SDL2 Library

### Compiling on Linux
```bash
# Install dependencies (Ubuntu/Debian-based)
sudo apt-get install libsdl2-dev cmake

# Clone the repository
git clone https://github.com/j4guil4r/MyGB.git
cd MyGB

# Build using CMake
mkdir build
cd build
cmake ..
cmake --build .

# Run the emulator
./MyGB path/to/rom.gb
```
### Running CPU Tests
This emulator uses Blargg's test ROMs to verify CPU instruction accuracy. To run the automated test suite:
```bash
cd build
ctest --output-on-failure
# Or run the test executable directly:
./MyGB_Tests
```