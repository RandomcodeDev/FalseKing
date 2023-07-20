#include "player.h"
#include "sprites.h"

flecs::entity Player::Create(flecs::world& world, Physics::State& physics)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);

    PxCapsuleControllerDesc controllerDesc;
    controllerDesc.setToDefault();
    controllerDesc.radius = 5;
    controllerDesc.height = 16;
    controllerDesc.material = material;

    flecs::entity player =
        world.entity("Player")
            .set(Physics::Controller(physics, controllerDesc))
            .set(Sprites::Player::player)
            .set(Components::MovementSpeed{0.75f, 0.5f, 1.25f})
            .set(Components::Health{100.0f, 100.0f})
            .set(Components::Element{Components::Element::None})
            .add<Tags::LocalPlayer>();

    material->release();

    return player;
}

flecs::entity Player::CreateProjectile(flecs::entity player, Physics::State& physics)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);

    flecs::entity projectile = 
        world.entity().set(Physics::Body(physics, ))
}

void Player::Input(flecs::iter& iter)
{
    Systems::Context* context = iter.ctx<Systems::Context>();
    Input::State* input = context->input;

    flecs::entity player = iter.entity(0);
    auto controller = player.get_mut<Physics::Controller>();
    auto movementSpeed = player.get<Components::MovementSpeed>();

    if (input->GetRightTrigger())
    {
    }

    float x = input->GetLeftStickDirection().x *
              (input->GetLeftStickPressed() ? movementSpeed->run
                                            : movementSpeed->walk) *
              (input->GetRightStickPressed() ? movementSpeed->crouch
                                             : movementSpeed->walk);
    float z = input->GetLeftStickDirection().y *
              (input->GetLeftStickPressed() ? movementSpeed->run
                                            : movementSpeed->walk) *
              (input->GetRightStickPressed() ? movementSpeed->crouch
                                             : movementSpeed->walk);

    controller->GetController().move(PxVec3(x, 0, z), 0.0001f,
                                     iter.delta_system_time() * 1000.0f,
                                     PxControllerFilters());
}
