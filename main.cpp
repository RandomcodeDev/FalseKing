#define QOI_IMPLEMENTATION

#include "backend.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"

Backend* g_backend;

// Update entities
static void Draw(ecs_iter_t* iter);

int GameMain(Backend* backend, std::vector<fs::path> backendPaths)
{
    g_backend = backend;

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SPDLOG_INFO("Initializing game");

    std::vector<fs::path> paths;
    paths.push_back("custom");
    for (const auto& path : backendPaths)
    {
        paths.push_back(path);
    }
    Filesystem::Initialize(paths);

    Image sprites(g_backend, "assets/sprites.qoi");

    ecs_world_t* world = ecs_init();
    ECS_COMPONENT(world, PhysicsController);
    ECS_COMPONENT(world, Sprite);
    ECS_SYSTEM(world, Draw, EcsOnUpdate, PhysicsController, Sprite);

    InputState input;
    PhysicsState physics;

    SPDLOG_INFO("Game initialized");

    PxMaterial* material =
        physics.GetPhysics().createMaterial(0.0f, 0.0f, 0.0f);

    PxRigidStatic* floorActor = physics.GetPhysics().createRigidStatic(
        PxTransform(PxVec3(0.0f, -1.0f, 0.0f)));
    PxShape* floorShape = PxRigidActorExt::createExclusiveShape(
        *floorActor, PxBoxGeometry(1000.0f, 0.5f, 1000.0f), *material);
    physics.GetScene().addActor(*floorActor);

    ecs_entity_t player = ecs_new_id(world);
    PxCapsuleControllerDesc playerControllerDesc;
    playerControllerDesc.setToDefault();
    playerControllerDesc.radius = Sprite::TILE_SIZE / 2;
    playerControllerDesc.height = 1.0f;
    playerControllerDesc.material = material;
    PxController* playerController =
        physics.GetControllerManager().createController(playerControllerDesc);
    ecs_set(world, player, PhysicsController, PhysicsController(playerController));
    ecs_set(world, player, Sprite, Sprite(sprites, 0, 0));

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    while (run)
    {
        now = precise_clock::now();
        chrono::milliseconds delta =
            chrono::duration_cast<chrono::milliseconds>(now - last);
        float floatDelta =
            (float)delta.count() / chrono::milliseconds::period::den;

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

        ecs_progress(world, floatDelta);

        bool canJump = playerController->getFootPosition().y -
                           floorActor->getGlobalPose().p.y <
                       0.6;
        playerController->move(
            PxVec3(input.GetLeftStickDirection().x,
                   input.GetA() && canJump ? 3.0f : -PhysicsState::GRAVITY,
                   input.GetLeftStickDirection().y),
            0.0f, floatDelta, PxControllerFilters());

        backend->EndRender();

        physics.Update(floatDelta);

        last = now;
        float ratio = 1.0f / 60.0f;
        if (floatDelta < ratio)
        {
            std::this_thread::sleep_for(chrono::milliseconds(
                (uint32_t)((ratio - floatDelta) * 1000.0f)));
        }
    }

    SPDLOG_INFO("Shutting down game");

    ecs_quit(world);

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}

static void Draw(ecs_iter_t* iter)
{
    PhysicsController& controller = *ecs_field(iter, PhysicsController, 1);
    Sprite& sprite = *ecs_field(iter, Sprite, 2);
    uint32_t x = (uint32_t)controller.GetTransform().p.x;
    uint32_t y = (uint32_t)std::max(
        (controller.GetTransform().p.z - controller.GetTransform().p.y), 0.0f);
    g_backend->DrawSprite(sprite, x, y);
}
