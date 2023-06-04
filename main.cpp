#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/chrono.h"

namespace chrono = std::chrono;
namespace fs = std::filesystem;
using precise_clock = chrono::high_resolution_clock;

#define GAME_NAME "Game" // TODO: come up with name

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

    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    if (SDL_CreateWindowAndRenderer(1024, 576, SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE, &window, &renderer) < 0)
    {
        Quit(fmt::format("Failed to create window or renderer: {}", SDL_GetError()), 1);
    }

    SPDLOG_INFO("Game initialized");

    WindowInfo info{};
    info.windowId = SDL_GetWindowID(window);
    SDL_GetWindowSize(window, &info.width, &info.height);
    info.focused = true;

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    chrono::milliseconds delta;
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

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        last = now;
    }

    SPDLOG_INFO("Shutting down game");

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
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
