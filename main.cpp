#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "box2d/box2d.h"

#include "entt/entt.hpp"

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/chrono.h"

namespace chrono = std::chrono;
namespace fs = std::filesystem;
using precise_clock = chrono::high_resolution_clock;

// Constants

constexpr const char* GAME_NAME = "Game"; // TODO: come up with name

// Physics time step
constexpr float PHYSICS_STEP = 1.0 / 60.0;

// Physics gravity
constexpr float PHYSICS_GRAVITY = 9.807;

// Physics iteration counts (these are the ones recommended in the docs)
constexpr int PHYSICS_VELOCITY_ITERATIONS = 8;
constexpr int PHYSICS_POSITION_ITERATIONS = 3;

// Exit the program
[[noreturn]]
void Quit(const std::string& message, int32_t exitCode);

// Window information
struct WindowInfo
{
    int32_t windowId;
    int32_t width;
    int32_t height;
    bool focused;
};

// Handle events
bool HandleEvent(const SDL_Event& event, WindowInfo& info);

// Update entities
void UpdateEntities(entt::registry& registry);

// Thing that can be rendered
class Renderable
{
public:
    Renderable(std::shared_ptr<SDL_Renderer> renderer, std::shared_ptr<Sprite> sprite)
        : renderer(renderer), sprite(sprite)
    {
    }
private:
    std::shared_ptr<SDL_Renderer> renderer;
    std::shared_ptr<Sprite> sprite;
};

// Transformation
struct Transform
{
    glm::vec2 position;
    glm::vec2 scale;
    float rotation;

    Transform()
        : position(), scale(), rotation()
    {}

    Transform(const glm::vec2& position, const glm::vec2& scale, float rotation)
        : position(position), scale(scale), rotation(rotation)
    {}
};

int SDL_main(int argc, char* argv[])
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SPDLOG_INFO("Initializing game");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        Quit(fmt::format("Failed to initialize SDL: {}", SDL_GetError()), 1);
    }

    SDL_Window* windowRaw = nullptr;
    SDL_Renderer* rendererRaw = nullptr;
    if (SDL_CreateWindowAndRenderer(1024, 576, SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE, &windowRaw, &rendererRaw) < 0)
    {
        Quit(fmt::format("Failed to create window or renderer: {}", SDL_GetError()), 1);
    }

    std::shared_ptr<SDL_Window> window(windowRaw);
    std::shared_ptr<SDL_Renderer> renderer(rendererRaw);

    entt::registry registry;

    b2World world(b2Vec2(0.0, -PHYSICS_GRAVITY));

    SPDLOG_INFO("Game initialized");

    WindowInfo info{};
    info.windowId = SDL_GetWindowID(window.get());
    SDL_GetWindowSize(window.get(), &info.width, &info.height);
    info.focused = true;

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    chrono::milliseconds delta;

    entt::entity player = registry.create();
    registry.emplace<Transform>(player, Transform(glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), 0.0));

    while (run)
    {
        SDL_Event event{};
        while (SDL_PollEvent(&event))
        {
            run = HandleEvent(event, info);
            if (!run)
            {
                break;
            }
        }

        if (!info.focused)
        {
            continue;
        }

        now = precise_clock::now();
        delta = chrono::duration_cast<chrono::milliseconds>(now - last);

        SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 0);
        SDL_RenderClear(renderer.get());

        world.Step(PHYSICS_STEP, PHYSICS_VELOCITY_ITERATIONS, PHYSICS_POSITION_ITERATIONS);
        UpdateEntities(registry);

        SDL_RenderPresent(renderer.get());

        last = now;
    }

    SPDLOG_INFO("Shutting down game");

    SDL_DestroyRenderer(renderer.get());
    SDL_DestroyWindow(window.get());
    SDL_Quit();

    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:%T}", now - start);

    return 0;
}

[[noreturn]]
void Quit(const std::string& message, int32_t exitCode)
{
    std::string title = fmt::format("Error {0}/0x{0:08X}", exitCode);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), NULL);
    exit(exitCode);
}

bool HandleEvent(const SDL_Event& event, WindowInfo& info)
{
    if (event.type >= SDL_EVENT_WINDOW_FIRST ||
        event.type <= SDL_EVENT_WINDOW_LAST &&
        event.window.windowID == info.windowId)
    {
        switch (event.window.type)
        {
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        {
            SPDLOG_INFO("Window focused");
            info.focused = true;
            return true;
        }
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            SPDLOG_INFO("Window unfocused");
            info.focused = false;
            return true;
        }
        case SDL_EVENT_WINDOW_RESIZED:
        {
            SPDLOG_INFO("Window resized from {}x{} to {}x{}", info.width,
                info.height, event.window.data1, event.window.data2);

            return true;
        }
        }
    }
    else if (event.type == SDL_EVENT_QUIT)
    {
        SPDLOG_INFO("Application quit");
        return false;
    }

    return true;
}

void UpdateEntities(entt::registry& registry)
{
    auto transformView = registry.view<Transform, std::shared_ptr<b2Body>>();
    auto renderableView = registry.view<Renderable>();
}
