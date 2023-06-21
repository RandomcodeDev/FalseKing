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

void Systems::DrawControlled(flecs::iter& iter, PhysicsController* controller,
                             const Sprite* sprite)
{
    uint32_t x = (uint32_t)controller->GetTransform().p.x;
    uint32_t y = (uint32_t)std::max(
        (controller->GetTransform().p.z - controller->GetTransform().p.y), 0.0f);
    g_backend->DrawSprite(*sprite, x, y);
}
