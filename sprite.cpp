#include "backend.h"
#include "sprite.h"

Sprite::Sprite(const Image& spriteSheet, uint32_t x, uint32_t y)
    : sheet(spriteSheet), x(x), y(y), width(SPRITE_SIZE), height(SPRITE_SIZE)
{
}

Sprite::Sprite(const Image& spriteSheet, uint32_t x,
    uint32_t y, uint32_t width, uint32_t height)
    : sheet(spriteSheet), x(x), y(y), width(width), height(height)
{
}
