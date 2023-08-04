#include "sprite.h"
#include "backend.h"
#include "camera.h"
#include "image.h"
#include "physics.h"
#include "systems.h"

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

void Systems::DrawPhysical(flecs::iter& iter)
{
    // TODO: Fix flickering
    auto context = iter.ctx<Systems::Context>();
    auto entity = iter.entity(0);
    auto sprite = entity.get<Sprite>();
    auto object = Physics::GetBase(entity);

    if (object && context->mainCamera->IsVisible(
            object->GetTransform().p,
            PxVec2((float)sprite->width, (float)sprite->height)))
    {
        PxVec2 position = context->mainCamera->Project(object->GetTransform().p);
        position.x -= sprite->width / 2;
        position.y -= sprite->height / 2;
        g_backend->DrawSprite(*sprite, (uint32_t)std::round(position.x),
                              (uint32_t)std::round(position.y));
    }
}
