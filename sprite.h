#pragma once

#include "game.h"
#include "image.h"

// Forward declarations
class Backend;

// Dimensions of one sprite tile
constexpr uint32_t SPRITE_SIZE = 16;

// Sprite
struct Sprite
{
    const Image& sheet;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;

    // Create a sprite
    Sprite(const Image& spriteSheet, uint32_t x, uint32_t y);

    // Create a sprite of arbitrary size
    Sprite(const Image& spriteSheet, uint32_t x, uint32_t y, uint32_t width,
           uint32_t height);
};
