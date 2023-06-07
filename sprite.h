#pragma once

#include "backend.h"
#include "game.h"
#include "image.h"

// Sprite
constexpr uint32_t SPRITE_SIZE = 16;
class Sprite
{
public:
    Sprite(const Image& spriteSheet, int32_t x, int32_t y)
        : x(x), y(y), width(1), height(1)
    {}

private:
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};
