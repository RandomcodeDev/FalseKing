#include "player.h"
#include "backend.h"
#include "sprites.h"

flecs::entity Player::Create(flecs::world& world, Physics::State& physics)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);

    PxCapsuleControllerDesc controllerDesc;
    controllerDesc.setToDefault();
    controllerDesc.radius = 0.5;
    controllerDesc.height = 2;
    controllerDesc.material = material;

    flecs::entity player =
        world.entity("Player")
            .set(Physics::Controller(physics, controllerDesc))
            .set(Sprite(Sprites::Player::player))
            .set(Components::MovementSpeed{0.75f, 0.5f, 1.25f})
            .set(Components::Health{100.0f, 100.0f})
            .set(Components::Element{Components::Element::None})
            .set(Cursor{0.0f, 0.0f})
            .set(MeleeCooldown{0.0f})
            .add<LocalPlayer>();

    material->release();

    return player;
}

flecs::entity Player::CreateProjectile(flecs::entity player,
                                       Physics::State& physics)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);
    PxSphereGeometry sphere(1);
    PxShape* shape = physics.GetPhysics().createShape(sphere, *material);

    PxTransform transform(GetCursorPosition(player));

    Physics::Body body = Physics::Body(physics, transform, *shape);

    const flecs::world& world = player.world();
    flecs::entity projectile =
        world.entity()
            .set(Components::Timeout{10.0f})
            .set(body)
            .set(*player.get<Components::Element>())
            .set(Sprite(Sprites::Player::fireMelee)) // TODO: make this actually
                                                     // respect the player's
                                                     // element and attack type
            .is_a<Tags::Projectile>()
            .child_of(player);
    material->release();

    // TODO: improve
    auto cursor = player.get<Cursor>();
    body.GetBody().addForce(PxVec3(cursor->x * 0.1f, 0, cursor->y * 0.1f));

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
    auto meleeCooldown = player.get_mut<MeleeCooldown>();

    meleeCooldown->value -= iter.delta_time();
    if (input->GetRightTrigger() && meleeCooldown->value <= 0.0f)
    {
        CreateProjectile(player, *context->physics);
        meleeCooldown->value = BASE_MELEE_COOLDOWN;
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

PxVec3 Player::GetCursorPosition(flecs::entity player, float distance)
{
    auto controller = player.get_mut<Physics::Controller>();
    auto cursor = player.get<Cursor>();

    float controllerX = controller->GetTransform().p.x;
    float controllerY = controller->GetTransform().p.y;
    float controllerZ = controller->GetTransform().p.z;
    float radius =
        ((PxCapsuleGeometry&)controller->GetShape(0)->getGeometry()).radius +
        distance;

    return PxVec3(controllerX + (cursor->x * radius), controllerY,
                  controllerZ + (cursor->y * radius));
}

// TODO: camera
void Player::DrawCursor(flecs::iter& iter)
{
    auto player = iter.entity(0);
    PxVec3 cursorPos(GetCursorPosition(player));
    uint32_t x = (uint32_t)cursorPos.x;
    uint32_t y = (uint32_t)(cursorPos.z - cursorPos.y);
    g_backend->DrawSprite(Sprites::Player::cursor, x, y);
}
