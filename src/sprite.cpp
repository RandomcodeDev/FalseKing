#include "sprite.h"
#include "backend.h"
#include "physics.h"

Sprite::Sprite(const Image& spriteSheet, uint32_t x, uint32_t y)
    : sheet(&spriteSheet), x(x), y(y), width(TILE_SIZE), height(TILE_SIZE)
{
}

Sprite::Sprite(const Image& spriteSheet, uint32_t x, uint32_t y, uint32_t width,
               uint32_t height)
    : sheet(&spriteSheet), x(x), y(y), width(width), height(height)
{
}

void Backend::DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y)
{
    if (sprite.sheet)
    {
        DrawImage(*sprite.sheet, x, y, 1.0f, 1.0f, sprite.x, sprite.y,
                  sprite.width, sprite.height);
    }
}

// TODO: replace with camera system

void Systems::DrawPhysical(Physics::Base& object, const Sprite& sprite)
{
    uint32_t x = (uint32_t)object.GetTransform().p.x;
    uint32_t y =
        (uint32_t)(object.GetTransform().p.z - object.GetTransform().p.y);

    g_backend->DrawSprite(sprite, x, y);
}
