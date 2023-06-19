#define QOI_IMPLEMENTATION

#include "backend.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"
#include "text.h"

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
    for (const auto& path : backendPaths)
    {
        paths.push_back(path);
    }
    Filesystem::Initialize(paths);
    Text::Initialize(backend);

    Image sprites(backend, "assets/sprites.qoi");

    ecs_world_t* world = ecs_init();

    InputState input;
    PhysicsState physics;

    SPDLOG_INFO("Game initialized");

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

        Text::DrawString(backend, "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890'\",.+-*/:;%$&?!", glm::uvec2(0));

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
