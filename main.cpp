#define QOI_IMPLEMENTATION

#include "backend.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"

// Update entities
static void UpdateEntities(Backend* backend, entt::registry& registry,
                           InputState& input, PhysicsState& physics);

int GameMain(Backend* backend, std::vector<fs::path> backendPaths)
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SPDLOG_INFO("Initializing game");

    std::vector<fs::path> paths;
    paths.push_back("custom");
    paths.append_range(backendPaths);
    Filesystem::Initialize(paths);

    Image sprites(backend, "assets/sprites.qoi");

    entt::registry registry;
    InputState input;
    PhysicsState physics;

    SPDLOG_INFO("Game initialized");

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    chrono::milliseconds delta;

    PxMaterial* material =
        physics.GetPhysics().createMaterial(1.0f, 1.0f, 1.0f);

    PxRigidStatic* floorActor = physics.GetPhysics().createRigidStatic(
        PxTransform(PxVec3(0.0f, -1.0f, 0.0f)));
    PxShape* floorShape = PxRigidActorExt::createExclusiveShape(
        *floorActor, PxBoxGeometry(50.0f, 0.5f, 50.0f), *material);
    physics.GetScene().addActor(*floorActor);

    entt::entity player = registry.create();
    PxRigidDynamic* playerActor =
        physics.GetPhysics().createRigidDynamic(PxTransform(PxVec3(0.0f)));
    PxShape* playerShape = PxRigidActorExt::createExclusiveShape(
        *playerActor, PxSphereGeometry(0.5f), *material);
    physics.GetScene().addActor(*playerActor);
    registry.emplace<PxRigidActor*>(player, playerActor);
    registry.emplace<Sprite>(player, Sprite(sprites, 0, 0));

    while (run)
    {
        now = precise_clock::now();
        delta = chrono::duration_cast<chrono::milliseconds>(now - last);

        if (!backend->Update(input))
        {
            break;
        }

        // Respect deadzones
        input.AdjustSticks();

        if (!backend->BeginRender())
        {
            continue;
        }

        UpdateEntities(backend, registry, input, physics);

        backend->EndRender();

        physics.Update(delta);

        last = now;
    }

    SPDLOG_INFO("Shutting down game");

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}

static void UpdateEntities(Backend* backend, entt::registry& registry,
                           InputState& input, PhysicsState& physics)
{
    auto view = registry.view<PxRigidActor*, Sprite>();

    for (auto& entity : view)
    {
        PxRigidActor* body = registry.get<PxRigidActor*>(entity);
        Sprite& sprite = registry.get<Sprite>(entity);
        uint32_t x = (uint32_t)body->getGlobalPose().p.x;
        uint32_t y =
            (uint32_t)(body->getGlobalPose().p.y + body->getGlobalPose().p.z);
        backend->DrawSprite(sprite, x, y);
    }
}
