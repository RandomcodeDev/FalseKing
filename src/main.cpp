#include "backend.h"
#include "components.h"
#include "discord.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"
#include "systems.h"
#include "text.h"

Backend* g_backend;

const char* GAME_COMMIT = {
#include "commit.txt"
};

int GameMain(Backend* backend, std::vector<std::string> backendPaths)
{
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last = start;

    g_backend = backend;

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SPDLOG_INFO("Initializing game");

    std::vector<std::string> paths;
    for (const auto& path : backendPaths)
    {
        paths.push_back(path);
    }
    Filesystem::Initialize(paths);
    Text::Initialize();
    Discord::Initialize();

    Image sprites("sprites.qoi");

    Input::State input;
    Physics::State physics;

    flecs::world world;
    // world.set_target_fps(60);

    Components::Register(world);

    Systems::Context context{&input, &physics, start};
    Systems::Register(world, &context);

    SPDLOG_INFO("Game initialized with backend {} on system {}",
                g_backend->DescribeBackend(), g_backend->DescribeSystem());

    PxMaterial* material =
        physics.GetPhysics().createMaterial(0.0f, 0.0f, 0.0f);

    PxCapsuleControllerDesc controllerDesc;
    controllerDesc.setToDefault();
    controllerDesc.radius = 5;
    controllerDesc.height = 16;
    controllerDesc.material = material;

    flecs::entity player =
        world.entity("Player")
            .set(Physics::Controller(physics, controllerDesc))
            .set(Sprite(sprites, 0, 0))
            .set(Components::MovementSpeed{0.75f, 0.5f, 1.25f})
            .set(Components::Health{100.0f, 100.0f})
            .add<Tags::Player>()
            .add<Tags::LocalPlayer>();

    bool run = true;
    while (run)
    {
        now = precise_clock::now();

        if (!g_backend->Update(input))
        {
            break;
        }

        // Respect deadzones
        input.AdjustSticks();

        world.progress();
        Discord::Update(
            chrono::duration_cast<chrono::seconds>(now - start),
            chrono::duration_cast<chrono::milliseconds>(now - last));

        last = now;
    }

    SPDLOG_INFO("Shutting down game");

    world.quit();
    Discord::Shutdown();
    Text::Shutdown();

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}
