#include "core/input.h"
#include "core/physics.h"

#include "components.h"
#include "game.h"
#include "player.h"

GAME_API void Game::Components::Register(flecs::world& world)
{
    ecs_os_init();
    world
        .component<Player::LocalPlayer>()
  //    .is_a(flecs::Private)
        .is_a<Player::Player>();
}
