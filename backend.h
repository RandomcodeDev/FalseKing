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

// Forward declarations
class Image;
struct Sprite;

// Abstraction of the platform
class Backend
{
public:
    // Set up an image so it can be used
    virtual void SetupImage(Image& image) = 0;

    // Clean up an image's resources
    virtual void CleanupImage(Image& image) = 0;

    // Process events
    virtual bool Update() = 0;

    // Prepare for rendering
    virtual bool BeginRender() = 0;
    
    // Draw an image
    virtual void DrawImage(const Image& image, uint32_t x, uint32_t y) = 0;
    
    // Draw a sprite
    virtual void DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y) = 0;

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
    void SetupImage(Image& image);
    void CleanupImage(Image& image);
    bool Update();
    bool BeginRender();
    void DrawImage(const Image& image, uint32_t x, uint32_t y);
    void DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y);
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
