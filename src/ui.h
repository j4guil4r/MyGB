#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include <array>
#include "bus.h"

class UI {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_AudioDeviceID audioDevice = 0;
    bool running = true;

public:
    UI();
    ~UI();

    bool init(int width, int height, int scale);
    void handleEvents(Bus& gbBus);
    void render(Bus& bus);
    void syncAudio(Bus& gbBus);
    bool isRunning() const { return running; }
};