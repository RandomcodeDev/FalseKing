#include "player.h"
#include "backend.h"
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
            .set(Cursor{0.0f, 0.0f})
            .add<LocalPlayer>();

    material->release();

    return player;
}

flecs::entity Player::CreateProjectile(flecs::entity player,
                                       Physics::State& physics)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);
    PxSphereGeometry sphere(9);
    PxShape* shape = physics.GetPhysics().createShape(sphere, *material);

    PxTransform transform(GetCursorPosition(player));

    Physics::Body body = Physics::Body(physics, transform, *shape);

    flecs::world& world = player.world();
    flecs::entity projectile = world.entity()
                                   .set(body)
                                   .set(Sprites::Player::fireMelee)
                                   .is_a<Tags::Projectile>()
                                   .child_of(player)
                                   .enable();
    material->release();

    // TODO: improve
    auto cursor = player.get<Cursor>();
    body.GetBody().addForce(PxVec3(cursor->x, 0, cursor->y));

    return projectile;
}

void Player::Input(flecs::iter& iter)
{
    Systems::Context* context = iter.ctx<Systems::Context>();
    Input::State* input = context->input;

    flecs::entity player = iter.entity(0);
    auto controller = player.get_mut<Physics::Controller>();
    auto movementSpeed = player.get<Components::MovementSpeed>();
    auto cursor = player.get_mut<Cursor>();

    if (input->GetRightTrigger())
    {
        CreateProjectile(player, *context->physics);
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

    cursor->x =
        std::clamp(cursor->x + input->GetRightStickDirection().x, -1.0f, 1.0f);
    cursor->y =
        std::clamp(cursor->y + input->GetRightStickDirection().y, -1.0f, 1.0f);

    controller->GetController().move(PxVec3(x, 0, z), 0.0001f,
                                     iter.delta_system_time() * 1000.0f,
                                     PxControllerFilters());
}

PxVec3 Player::GetCursorPosition(flecs::entity player)
{
    auto controller = player.get_mut<Physics::Controller>();
    auto cursor = player.get<Cursor>();

    float controllerX = controller->GetTransform().p.x;
    float controllerY = controller->GetTransform().p.y;
    float controllerZ = controller->GetTransform().p.z;
    float radius =
        ((PxCapsuleGeometry&)controller->GetShape(0)->getGeometry()).radius + 3;

    return PxVec3(controllerX + (cursor->x * radius), controllerY,
                  controllerZ + (cursor->y * radius));
}

// TODO: camera
void Player::DrawCursor(flecs::iter& iter)
{
    auto player = iter.entity(0);
    PxVec3 cursorPos(GetCursorPosition(player));
    uint32_t x = cursorPos.x;
    uint32_t y = cursorPos.z - cursorPos.y;
    g_backend->DrawSprite(Sprites::Player::cursor, x, y);
}
