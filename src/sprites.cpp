#include "sprites.h"
#include "image.h"

namespace Sprites
{
static Image* s_sprites;
void Load()
{
    SPDLOG_INFO("Loading sprites");

    s_sprites = new Image("sprites.qoi");

    Player::player = Sprite(*s_sprites, 5, 1, 5, 15);
    Player::cursor = Sprite(*s_sprites, 5, 22, 5, 5);
    Player::fireMelee = Sprite(*s_sprites, 20, 4, 9, 9);
}

void Unload()
{
    if (s_sprites)
    {
        delete s_sprites;
        s_sprites = nullptr;
    }
}

namespace Player
{
Sprite player;
Sprite cursor;
Sprite fireMelee;
} // namespace Player
} // namespace Sprites
