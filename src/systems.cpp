#include "systems.h"

void Systems::Register(flecs::world& world, InputState* input)
{
    // systems.h
    world.system<const Components::LocalPlayer>()
        .ctx(input)
        .kind(flecs::OnUpdate)
        .iter(PlayerInput);

    // sprite.h
    world.system<PhysicsController, const Sprite>()
        .kind(flecs::OnUpdate)
        .iter(DrawControlled);
}

void Systems::PlayerInput(flecs::iter& iter)
{
    InputState* input = iter.ctx<InputState>();
    flecs::entity player = iter.entity(0);
    auto controller = player.get_mut<PhysicsController>();
    auto movementSpeed = player.get<Components::MovementSpeed>();
    float x = input->GetLeftStickDirection().x * (input->GetLeftStickPressed()
                  ? movementSpeed->run
                  : movementSpeed->walk);
    float z = input->GetLeftStickDirection().y * (input->GetLeftStickPressed()
                  ? movementSpeed->run
                  : movementSpeed->walk);
    controller->GetController().move(PxVec3(x, 0, z),
                                    0.0001f, iter.delta_time() * 1000.0f,
                                    PxControllerFilters());
}
