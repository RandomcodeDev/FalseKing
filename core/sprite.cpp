#include "sprite.h"
#include "backend.h"
#include "camera.h"
#include "image.h"
#include "physics.h"
#include "systems.h"

CORE_API Core::Sprite::Sprite(std::shared_ptr<Image> spriteSheet, uint32_t x, uint32_t y)
    : sheet(spriteSheet), x(x), y(y), width(TILE_SIZE), height(TILE_SIZE)
{
}

CORE_API Core::Sprite::Sprite(std::shared_ptr<Image> spriteSheet, uint32_t x, uint32_t y,
                              uint32_t width,
               uint32_t height)
    : sheet(spriteSheet), x(x), y(y), width(width), height(height)
{
}

CORE_API void Core::Backend::DrawSprite(Sprite& sprite, uint32_t x, uint32_t y,
                         bool center)
{
    if (sprite.sheet)
    {
        DrawImage(*sprite.sheet, x - (center ? sprite.width / 2 : 0),
                  y - (center ? sprite.height / 2 : 0), 1.0f, 1.0f, sprite.x,
                  sprite.y, sprite.width, sprite.height);
    }
}

CORE_API void Core::Systems::DrawPhysical(flecs::iter& iter)
{
    // TODO: Fix flickering
    auto context = iter.ctx<Systems::Context>();
    auto entity = iter.entity(0);
    auto sprite = entity.get_mut<Sprite>();
    auto object = Physics::GetBase(entity);

    if (object && context->mainCamera->IsVisible(
                      object->GetTransform().p,
                      PxVec2((float)sprite->width, (float)sprite->height)))
    {
        PxVec2 position =
            context->mainCamera->Project(object->GetTransform().p);
        g_backend->DrawSprite(*sprite, (uint32_t)std::round(position.x),
                              (uint32_t)std::round(position.y));
    }
}
