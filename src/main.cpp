#include "backend.h"
#include "components.h"
#include "discord.h"
#include "fs.h"
#include "game.h"
#include "image.h"
#include "input.h"
#include "physics.h"
#include "player.h"
#include "sprite.h"
#include "sprites.h"
#include "systems.h"

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
    Discord::Initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    std::vector<uint8_t> font = Filesystem::Read("font.ttf");
    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    
    io.Fonts->AddFontFromMemoryTTF(font.data(), font.size(), 4.0f, &fontConfig);

    g_backend->InitializeImGui();

    Sprites::Load();

    Input::State input;
    Physics::State physics;

    flecs::world world;
#ifdef _DEBUG
#ifdef FLECS_REST
    world.set<flecs::Rest>({});
#endif
    world.import<flecs::monitor>();
#endif
    // world.set_target_fps(60);

    Systems::Context context{&input, &physics, start};
    Components::Register(world);
    Systems::Register(world, &context);

    SPDLOG_INFO("Game initialized with backend {} on system {}",
                g_backend->DescribeBackend(), g_backend->DescribeSystem());

    PxMaterial* material =
        physics.GetPhysics().createMaterial(0.0f, 0.0f, 0.0f);

    Player::Create(world, physics);

    bool run = true;
    while (run)
    {
        now = precise_clock::now();

        if (!g_backend->Update(input))
        {
            break;
        }

        if (!g_backend->GetWindowInformation().focused)
        {
            world.set_target_fps(10);
        }
        else
        {
            if (world.get_target_fps() == 10)
            {
                world.set_target_fps(0);
            }
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

    Sprites::Unload();

    g_backend->ShutdownImGui();
    ImGui::DestroyContext();

    Discord::Shutdown();

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}
