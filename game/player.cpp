#include "core/backend.h"
#include "core/camera.h"
#include "core/components.h"
#include "core/input.h"
#include "core/physics.h"
#include "core/systems.h"

#include "components.h"
#include "player.h"
#include "sprites.h"
#include "systems.h"

GAME_API flecs::entity Game::Player::Create(flecs::world& world,
                                   Core::Physics::State& physics,
                                   Core::Components::Camera** camera)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);

    PxCapsuleControllerDesc controllerDesc;
    controllerDesc.setToDefault();
    controllerDesc.radius = 0.5;
    controllerDesc.height = 2;
    controllerDesc.material = material;

    flecs::entity player =
        world.entity("Player")
            .set(Core::Physics::Controller(physics, controllerDesc))
            .set(Core::Sprite(Game::Sprites::Player::player))
            .set(Core::Components::MovementSpeed{0.75f, 0.5f, 1.25f})
            .set(Core::Components::Health{100.0f, 100.0f})
            .set(Components::Element{Components::Element::None})
            .set(Cursor{0.0f, 0.0f})
            .set(MeleeCooldown{0.0f})
            .set(**camera)
            .add<LocalPlayer>();

    *camera = player.get_mut<Core::Components::Camera>();

    material->release();

    return player;
}

GAME_API flecs::entity Game::Player::CreateProjectile(
    flecs::entity player,
                                             Core::Physics::State& physics,
                                             float lifespan, float speed)
{
    PxMaterial* material = physics.GetPhysics().createMaterial(0, 0, 0);
    PxSphereGeometry sphere(1);
    PxShape* shape = physics.GetPhysics().createShape(sphere, *material);

    PxTransform transform(GetCursorPosition(player));

    Core::Physics::Body body = Core::Physics::Body(physics, transform, *shape);
    body.GetBody().setMass(0.0f);

    const flecs::world& world = player.world();
    flecs::entity projectile =
        world.entity()
            .set(Core::Components::Timeout{lifespan})
            .set(body)
            .set(*player.get<Components::Element>())
            .set(Core::Sprite(
                Game::Sprites::Player::fireMelee)) // TODO: make this actually
                                                   // respect the player's
                                                   // element and attack type
            .is_a<Core::Tags::Projectile>()
            .child_of(player);
    material->release();

    // TODO: improve
    auto cursor = player.get<Cursor>();
    body.GetBody().addForce(PxVec3(cursor->x * speed, 0, cursor->y * speed));

    return projectile;
}

GAME_API void Game::Player::HandleInput(flecs::iter& iter)
{
    auto context = iter.ctx<Core::Systems::Context>();
    auto input = context->input;

    flecs::entity player = iter.entity(0);
    auto controller = player.get_mut<Core::Physics::Controller>();
    auto movementSpeed = player.get<Core::Components::MovementSpeed>();
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

GAME_API PxVec3 Game::Player::GetCursorPosition(flecs::entity player,
                                                float distance)
{
    auto controller = player.get_mut<Core::Physics::Controller>();
    auto cursor = player.get<Cursor>();

    float radius =
        ((PxCapsuleGeometry&)controller->GetShape(0)->getGeometry()).radius +
        distance;
    PxVec3 position = controller->GetTransform().p;
    float angle = PxAtan2(cursor->y, cursor->x);

    return PxVec3(position.x + radius * PxCos(angle), position.y,
                  position.z + radius * PxSin(angle));
}

GAME_API void Game::Player::DrawCursor(flecs::iter& iter)
{
    auto player = iter.entity(0);
    auto camera = player.get<Core::Components::Camera>();
    PxVec3 cursorPosition(GetCursorPosition(player));
    PxVec2 screenPosition = camera->Project(cursorPosition);
    Core::g_backend->DrawSprite(Game::Sprites::Player::cursor,
                                (uint32_t)screenPosition.x,
                                (uint32_t)screenPosition.y);
}
