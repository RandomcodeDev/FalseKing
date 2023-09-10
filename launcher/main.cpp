#include "core/backend.h"
#include "core/camera.h"
#include "core/components.h"
#include "core/discord.h"
#include "core/fs.h"
#include "core/image.h"
#include "core/input.h"
#include "core/physics.h"
#include "core/sprite.h"
#include "core/core.h"
#include "core/systems.h"

#include "game/components.h"
#include "game/player.h"
#include "game/sprites.h"
#include "game/systems.h"

void embraceTheDarkness();

int32_t Launcher::GameMain(Core::Backend* backend, std::vector<std::string> backendPaths)
{
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last = start;
    chrono::time_point<precise_clock> lastOverlayCycle = start;

    Core::g_backend = backend;

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
    paths.push_back("assets.vpk");
    paths.push_back("assets");
    Core::Filesystem::Initialize(paths);
    Core::Discord::Initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    embraceTheDarkness();
    ImGuiStyle style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

    std::vector<uint8_t> font = Core::Filesystem::Read("fonts/monocraft.ttf");

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(font.data(), (int32_t)font.size(), 64.0f,
                                   &fontConfig);
    io.FontGlobalScale = 0.18f;

    Core::g_backend->InitializeImGui();

    ImGui::SetCurrentContext(GImGui);

    Core::Input::State input;
    Core::Physics::State physics;

    ecs_os_init();

    flecs::world world;
#ifdef _DEBUG
#ifdef FLECS_REST
    world.set<flecs::Rest>({});
#endif
    world.import <flecs::monitor>();
#endif
    // world.set_target_fps(60);

    Core::Components::Camera camera;

    Core::Systems::Context context = {
        start,    // start time
        &input,   // input state
        &physics, // physics state
        &camera,  // main camera

    // Whether to enable the debug overlay initially
#ifdef RETAIL
        Core::Systems::DebugMode::None,
#else
        Core::Systems::DebugMode::All,
#endif
    };

    Core::Components::Register(world);
    Core::Systems::Register(world, &context);

    Game::Sprites::Load();
    Game::Components::Register(world);
    Game::Systems::Register(world, &context);

    Game::Player::Create(world, physics, &context.mainCamera);

    SPDLOG_INFO("Game initialized with backend {} on system {}",
                Core::g_backend->DescribeBackend(),
                Core::g_backend->DescribeSystem());

    PxMaterial* material =
        physics.GetPhysics().createMaterial(0.0f, 0.0f, 0.0f);

    const Core::WindowInfo& windowInfo =
        Core::g_backend->GetWindowInformation();

    bool run = true;
    while (run)
    {
        now = precise_clock::now();

        if (!Core::g_backend->Update(input))
        {
            break;
        }

        if (!windowInfo.focused)
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

        if (windowInfo.resized)
        {
            float scale = (float)windowInfo.width / (float)windowInfo.lastWidth;
            for (const auto& viewport : ImGui::GetCurrentContext()->Viewports)
            {
                ImGui::ScaleWindowsInViewport(viewport, scale);
            }
            ImGui::GetStyle().ScaleAllSizes(scale);
            ImGui::GetIO().FontGlobalScale *= scale;
        }

        // Keep inputs within functional range
        input.AdjustSticks();

        // Check if the debug mode was cycled
        if (input.GetDebugCycled() &&
            now - lastOverlayCycle > Core::Systems::DEBUG_CYCLE_COOLDOWN)
        {
            context.debugMode =
                (Core::Systems::DebugMode)(((uint32_t)context.debugMode << 1) %
                (uint32_t)Core::Systems::DebugMode::All);
            if (context.debugMode > Core::Systems::DebugMode::Count)
            {
                context.debugMode = Core::Systems::DebugMode::All;
            }
            lastOverlayCycle = precise_clock::now();
        }

        world.progress();
        Core::Discord::Update(
            chrono::duration_cast<chrono::seconds>(now - start),
            chrono::duration_cast<chrono::milliseconds>(now - last));

        last = now;
    }

    SPDLOG_INFO("Shutting down game");

    world.quit();

    Game::Sprites::Unload();

    Core::g_backend->ShutdownImGui();
    ImGui::DestroyContext();

    Core::Discord::Shutdown();

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}

// https://github.com/ocornut/imgui/issues/707
void embraceTheDarkness()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] =
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] =
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    // colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    // colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 0;
    style.ChildRounding = 0;
    style.FrameRounding = 0;
    style.PopupRounding = 0;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 0;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 0;
}
