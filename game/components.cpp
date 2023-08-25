#include "components.h"
#include "input.h"
#include "physics.h"
#include "player.h"

void Game::Components::Register(flecs::world& world)
{
    world.component<Player::LocalPlayer>()
    //    .is_a(flecs::Private)
        .is_a<Player::Player>();
}
