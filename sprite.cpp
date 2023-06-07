#include "backend.h"
#include "sprite.h"

Sprite::Sprite(Backend* backend, const Image& spriteSheet, uint32_t x, uint32_t y)
{
    Sprite(backend, spriteSheet, x, y, SPRITE_SIZE, SPRITE_SIZE);
}

Sprite::Sprite(Backend* backend, const Image& spriteSheet, uint32_t x,
    uint32_t y, uint32_t width, uint32_t height)
    : m_backend(backend), m_x(x), m_y(y), m_width(width), m_height(height)
{
    m_backend->SetupSprite(spriteSheet, *this);
}

Sprite::~Sprite()
{
    m_backend->CleanupSprite(*this);
}
