#pragma once

#include "game.h"
#include "image.h"

// Forward declarations
class Backend;

// Sprite
struct Sprite
{
    const Image* sheet;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;

    // Dimensions of one sprite tile
    static constexpr uint32_t TILE_SIZE = 16;

    Sprite() = default;

    // Create a sprite
    Sprite(const Image& spriteSheet, uint32_t x, uint32_t y);

    // Create a sprite of arbitrary size
    Sprite(const Image& spriteSheet, uint32_t x, uint32_t y, uint32_t width,
           uint32_t height);
};

struct PhysicsController;

namespace Systems
{
// Draw a sprite with a physics controller
void DrawControlled(flecs::iter& iter, PhysicsController* controller,
                    const Sprite* sprite);
} // namespace Systems
