#include "player.h"
#include "backend.h"
#include "camera.h"
#include "components.h"
#include "input.h"
#include "physics.h"
#include "sprites.h"
#include "systems.h"

flecs::entity Player::Create(flecs::world& world, Physics::State& physics,
                             Components::Camera** camera)
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
            .set(**camera)
            .add<LocalPlayer>();

    *camera = player.get_mut<Components::Camera>();

    material->release();

    return player;
}

flecs::entity Player::CreateProjectile(flecs::entity player,
                                       Physics::State& physics, float lifespan,
                                       float speed)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);
    PxSphereGeometry sphere(1);
    PxShape* shape = physics.GetPhysics().createShape(sphere, *material);

    PxTransform transform(GetCursorPosition(player));

    Physics::Body body = Physics::Body(physics, transform, *shape);
    body.GetBody().setMass(0.0f);

    const flecs::world& world = player.world();
    flecs::entity projectile =
        world.entity()
            .set(Components::Timeout{lifespan})
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
    body.GetBody().addForce(PxVec3(cursor->x * speed, 0, cursor->y * speed));

    return projectile;
}

void Player::HandleInput(flecs::iter& iter)
{
    auto context = iter.ctx<Systems::Context>();
    auto input = context->input;

    flecs::entity player = iter.entity(0);
    auto controller = player.get_mut<Physics::Controller>();
    auto movementSpeed = player.get<Components::MovementSpeed>();
    auto cursor = player.get_mut<Cursor>();
    auto meleeCooldown = player.get_mut<MeleeCooldown>();

    // TODO: make timing of this based on animation
    meleeCooldown->value -= iter.delta_time();
    if (input->GetRightTrigger() && meleeCooldown->value <= 0.0f)
    {
        CreateProjectile(player, *context->physics, 0.3f, 3.0f);
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

    float radius =
        ((PxCapsuleGeometry&)controller->GetShape(0)->getGeometry()).radius +
        distance;
    PxVec3 position = controller->GetTransform().p;
    float angle = PxAtan2(cursor->y, cursor->x);

    return PxVec3(position.x + radius * PxCos(angle), position.y,
                  position.z + radius * PxSin(angle));
}

void Player::DrawCursor(flecs::iter& iter)
{
    auto player = iter.entity(0);
    auto camera = player.get<Components::Camera>();
    PxVec3 cursorPosition(GetCursorPosition(player));
    PxVec2 screenPosition = camera->Project(cursorPosition);
    g_backend->DrawSprite(Sprites::Player::cursor, (uint32_t)screenPosition.x,
                          (uint32_t)screenPosition.y);
}
