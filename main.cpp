#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "SDL3/SDL.h"
#include "SDL3/sdl_main.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"

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
    double now = 0;
    double last = 0;
    double delta = 0;
    double runtime = 0;
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

        now = (double)SDL_GetTicksNS() / 1000000;
        delta = now - last;
        runtime += delta;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        last = now;
    }

    SPDLOG_INFO("Shutting down game");


    SPDLOG_INFO("Game shut down");
    SPDLOG_INFO("Game ran for {:.3} seconds", runtime / 1000);

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
