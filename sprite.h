#pragma once

#include "game.h"
#include "image.h"

// Forward declarations
class Backend;

// Dimensions of one sprite tile
constexpr uint32_t SPRITE_SIZE = 16;

// Sprite
class Sprite
{
public:
    // Opaque backend information
    void* backendData;

    // Create a sprite
    Sprite(Backend* backend, const Image& spriteSheet, uint32_t x, uint32_t y);

    // Create a sprite of arbitrary size
    Sprite(Backend* backend, const Image& spriteSheet, uint32_t x, uint32_t y,
        uint32_t width, uint32_t height);

    // Clean up the sprite
    ~Sprite();

    // Get the size of the sprite
    void GetSize(uint32_t& width, uint32_t& height) const
    {
        width = m_width;
        height = m_height;
    }
private:
    Backend* m_backend;
    uint32_t m_x;
    uint32_t m_y;
    uint32_t m_width;
    uint32_t m_height;
};
