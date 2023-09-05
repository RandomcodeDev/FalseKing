#pragma once

#include "stdafx.h"

namespace Core
{

// Forward declarations
class CORE_API Backend;
class CORE_API Image;

// Sprite
struct CORE_API Sprite
{
    std::shared_ptr<Image> sheet;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;

    // Dimensions of one sprite tile
    static constexpr uint32_t TILE_SIZE = 16;

    Sprite() = default;

    // Create a sprite
    Sprite(std::shared_ptr<Image> spriteSheet, uint32_t x, uint32_t y);

    // Create a sprite of arbitrary size
    Sprite(std::shared_ptr<Image> spriteSheet, uint32_t x, uint32_t y,
           uint32_t width,
           uint32_t height);
};

namespace Systems
{
struct Context;

// Draw a sprite with a physics thing
CORE_API void DrawPhysical(flecs::iter& iter);
} // namespace Systems

} // namespace Core