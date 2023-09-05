#include "core/image.h"

#include "game.h"
#include "sprites.h"

namespace Game
{
namespace Sprites
{

static std::shared_ptr<Core::Image> s_sprites;
GAME_API void Load()
{
    SPDLOG_INFO("Loading sprites");

    s_sprites = std::make_shared<Core::Image>("textures/sprites.qoi");

    Player::player = Core::Sprite(s_sprites, 5, 1, 5, 15);
    Player::cursor = Core::Sprite(s_sprites, 5, 22, 5, 5);
    Player::fireMelee = Core::Sprite(s_sprites, 20, 4, 9, 9);
}

GAME_API void Unload()
{
    if (s_sprites)
    {
        s_sprites.reset();
    }
}

namespace Player
{
GAME_API Core::Sprite player;
GAME_API Core::Sprite cursor;
GAME_API Core::Sprite fireMelee;
} // namespace Player

} // namespace Sprites
} // namespace Game