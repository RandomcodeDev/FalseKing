#pragma once

#include "game.h"

// Window information
struct WindowInfo
{
    void* handle;
    int32_t width;
    int32_t height;
    bool focused;
};

// Abstraction of the platform
class Backend
{
public:
    // Process events
    virtual bool Update() = 0;

    // Prepare for rendering
    virtual bool BeginRender() = 0;

    // Complete rendering
    virtual void EndRender() = 0;

    // Get window information
    virtual WindowInfo* GetWindowInformation() = 0;
};

#ifdef USE_SDL
#include "SDL3/SDL.h"

class SdlBackend : Backend
{
public:
    SdlBackend();
    ~SdlBackend();
    bool Update();
    bool BeginRender();
    void EndRender();
    WindowInfo* GetWindowInformation()
    {
        return &m_windowInfo;
    }

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    WindowInfo m_windowInfo;
    int32_t m_windowId;

    bool HandleEvent(const SDL_Event& event);
};
#endif

// Program entry point
extern int GameMain(Backend* backend);
