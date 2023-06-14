#define QOI_IMPLEMENTATION

#include "backend.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"

// Update entities
static void UpdateEntities(Backend* backend, entt::registry& registry, InputState& input, PhysicsState& physics);

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

    entt::entity player = registry.create();
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
    auto view = registry.view<Sprite>();

    for (auto& entity : view)
    {
        Sprite& sprite = registry.get<Sprite>(entity);
        backend->DrawSprite(sprite, (uint32_t)0,
                            (uint32_t)0);
    }
}
