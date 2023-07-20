#include "components.h"
#include "sprite.h"

void Components::Register(flecs::world& world)
{
    world.component<Tags::LocalPlayer>()
        .is_a(flecs::Private)
        .is_a<Tags::Player>();
}
