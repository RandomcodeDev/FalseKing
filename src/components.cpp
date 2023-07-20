#include "components.h"
#include "player.h"

void Components::Register(flecs::world& world)
{
    world.component<Player::LocalPlayer>()
    //    .is_a(flecs::Private)
        .is_a<Player::Player>();
}
