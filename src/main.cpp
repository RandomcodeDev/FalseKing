#include "backend.h"
#include "components.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"
#include "systems.h"
#include "text.h"

Backend* g_backend;

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
    Text::Initialize();

    Image sprites("assets/sprites.qoi");

    InputState input;
    PhysicsState physics;

    flecs::world world;
    Systems::Register(world, &input);

    SPDLOG_INFO("Game initialized");

    PxMaterial* material =
        physics.GetPhysics().createMaterial(0.0f, 0.0f, 0.0f);

    PxCapsuleControllerDesc controllerDesc;
    controllerDesc.setToDefault();
    controllerDesc.radius = Sprite::TILE_SIZE / 2;
    controllerDesc.height = 1.0f;
    controllerDesc.material = material;

    flecs::entity player = world.entity("Player")
                               .set(PhysicsController(physics, controllerDesc))
                               .set(Sprite(sprites, 0, 0))
                               .set(Components::MovementSpeed{0.5f, 0.75f})
                               .set(Components::Health{100.0f, 100.0f})
                               .add<Components::LocalPlayer>();

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    float fps = 0.0f;
    while (run)
    {
        now = precise_clock::now();
        chrono::milliseconds delta =
            chrono::duration_cast<chrono::milliseconds>(now - last);
        float floatDelta =
            (float)delta.count() / chrono::milliseconds::period::den;
        if (delta.count() > 0)
        {
            fps = (fps * FRAME_SMOOTHING) +
                  ((1000.0f / delta.count()) * (1.0f - FRAME_SMOOTHING));
        }

        if (!g_backend->Update(input))
        {
            break;
        }

        // Respect deadzones
        input.AdjustSticks();

        if (!g_backend->BeginRender())
        {
            continue;
        }

        world.progress(floatDelta);

        Text::DrawString(
            fmt::format("FPS: {:0.3}\nFrame delta: {}", fps, delta),
            glm::uvec2(0), 0.3f, glm::u8vec3(0, 255, 0));

        g_backend->EndRender();

        physics.Update(floatDelta);

        last = now;
        const float ratio = 1.0f / 60.0f;
        while (chrono::duration_cast<chrono::duration<float, std::milli>>(now -
                                                                          last)
                   .count() <= ratio * 1000.0f)
        {
            now = precise_clock::now();
        }
    }

    SPDLOG_INFO("Shutting down game");

    Text::Shutdown();

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}
