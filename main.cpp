#include "backend.h"
#include "game.h"
#include "image.h"
#include "mathutil.h"
#include "sprite.h"

// Update entities
static void UpdateEntities(entt::registry& registry);

int GameMain(Backend* backend)
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SPDLOG_INFO("Initializing game");

    entt::registry registry;

    b2World world(b2Vec2(0.0, -PHYSICS_GRAVITY));

    SPDLOG_INFO("Game initialized");

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    chrono::milliseconds delta;

    entt::entity player = registry.create();
    registry.emplace<Transform>(player, Transform(glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), 0.0));

    while (run)
    {
        now = precise_clock::now();
        delta = chrono::duration_cast<chrono::milliseconds>(now - last);

        if (!backend->BeginRender())
        {
            continue;
        }

        world.Step(PHYSICS_STEP, PHYSICS_VELOCITY_ITERATIONS, PHYSICS_POSITION_ITERATIONS);
        UpdateEntities(registry);

        backend->EndRender();

        last = now;
    }

    SPDLOG_INFO("Shutting down game");

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}

static void UpdateEntities(entt::registry& registry)
{
    auto transformView = registry.view<Transform, b2Body*>();
    auto spriteView = registry.view<Sprite>();
}
