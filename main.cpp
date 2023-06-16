#define QOI_IMPLEMENTATION

#include "backend.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"

// Update entities
static void UpdateEntities(Backend* backend, entt::registry& registry);

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

    PxMaterial* material =
        physics.GetPhysics().createMaterial(0.0f, 0.0f, 0.0f);

    PxRigidStatic* floorActor = physics.GetPhysics().createRigidStatic(
        PxTransform(PxVec3(0.0f, -1.0f, 0.0f)));
    PxShape* floorShape = PxRigidActorExt::createExclusiveShape(
        *floorActor, PxBoxGeometry(1000.0f, 0.5f, 1000.0f), *material);
    physics.GetScene().addActor(*floorActor);

    entt::entity player = registry.create();
    PxCapsuleControllerDesc playerControllerDesc;
    playerControllerDesc.setToDefault();
    playerControllerDesc.radius = Sprite::TILE_SIZE / 2;
    playerControllerDesc.height = 1.0f;
    playerControllerDesc.material = material;
    PxController* playerController =
        physics.GetControllerManager().createController(playerControllerDesc);
    registry.emplace<PxController*>(player, playerController);
    registry.emplace<Sprite>(player, Sprite(sprites, 0, 0));

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    while (run)
    {
        now = precise_clock::now();
        chrono::milliseconds delta = chrono::duration_cast<chrono::milliseconds>(now - last);
        float floatDelta = (float)delta.count() / chrono::milliseconds::period::den;

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

        UpdateEntities(backend, registry);
        
        SPDLOG_INFO("{} {} {}", input.GetLeftStickDirection().x, input.GetLeftStickDirection().y, floatDelta);
        bool canJump = playerController->getFootPosition().y - floorActor->getGlobalPose().p.y < 0.6;
        playerController->move(
            PxVec3(input.GetLeftStickDirection().x, input.GetA() && canJump ? 3.0f : -PhysicsState::GRAVITY,
                   input.GetLeftStickDirection().y),
            0.0f, floatDelta,
            PxControllerFilters());

        backend->EndRender();

        physics.Update(floatDelta);
        
        last = now;
        float ratio = 1.0f / 60.0f;
        if (floatDelta < ratio)
        {
            std::this_thread::sleep_for(chrono::milliseconds((uint32_t)((ratio - floatDelta) * 1000.0f)));
        }
    }

    SPDLOG_INFO("Shutting down game");

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}

static void UpdateEntities(Backend* backend, entt::registry& registry)
{
    auto playerView = registry.view<PxController*, Sprite>();
    auto view = registry.view<PxRigidActor*, Sprite>();

    for (auto& player : playerView)
    {
        PxController* controller = registry.get<PxController*>(player);
        Sprite& sprite = registry.get<Sprite>(player);
        uint32_t x = (uint32_t)controller->getPosition().x;
        uint32_t y = (uint32_t)std::max((controller->getPosition().z -
                                controller->getPosition().y), 0.0);
        SPDLOG_INFO("{} {}", x, y);
        backend->DrawSprite(sprite, x, y);
    }

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
