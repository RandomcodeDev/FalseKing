#define QOI_IMPLEMENTATION

#include "backend.h"
#include "game.h"
#include "image.h"
#include "mathutil.h"
#include "sprite.h"

// Update entities
static void UpdateEntities(Backend* backend, entt::registry& registry);

int GameMain(Backend* backend)
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SPDLOG_INFO("Initializing game");

    Image sprites(backend, "assets/sprites.qoi");

    entt::registry registry;

    b2World world(b2Vec2(0.0, -PHYSICS_GRAVITY));

    SPDLOG_INFO("Game initialized");

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    chrono::milliseconds delta;

    entt::entity player = registry.create();
    registry.emplace<Transform>(player, Transform(glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), 0.0));
    registry.emplace<Sprite>(player, Sprite(sprites, 0, 0));

    while (run)
    {
        now = precise_clock::now();
        delta = chrono::duration_cast<chrono::milliseconds>(now - last);

        if (!backend->Update())
        {
            break;
        }

        if (!backend->BeginRender())
        {
            continue;
        }

        world.Step(PHYSICS_STEP, PHYSICS_VELOCITY_ITERATIONS, PHYSICS_POSITION_ITERATIONS);
        UpdateEntities(backend, registry);

        backend->EndRender();

        last = now;
    }

    SPDLOG_INFO("Shutting down game");

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}

static void UpdateEntities(Backend* backend, entt::registry& registry)
{
    auto transformView = registry.view<Transform, b2Body*>();
    auto spriteView = registry.view<Transform, Sprite>();

    for (auto& entity: spriteView)
    {
        Transform& transform = registry.get<Transform>(entity);
        Sprite& sprite = registry.get<Sprite>(entity);
        backend->DrawSprite(sprite, transform.position.x, transform.position.y);
    }
}
