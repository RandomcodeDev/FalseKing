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

// Maps backend keycodes to inputs
union KeyMapping
{
    struct
    {
        uint32_t esc; // Start
        uint32_t tab; // Select
        uint32_t q; // Up
        uint32_t c; // Down
        uint32_t x; // Left
        uint32_t v; // Right
        uint32_t space; // A
        uint32_t f; // B
        uint32_t e; // X
        uint32_t r; // Y
        uint32_t scrollUp; // Left shoulder
        uint32_t scrollDown; // Right shoulder
        uint32_t shift; // Left stick
        uint32_t control; // Right stick
        uint32_t w;
        uint32_t s;
        uint32_t a;
        uint32_t d;
    };
    uint32_t values[18];
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
    virtual bool Update(class InputState& input) = 0;

    // Prepare for rendering
    virtual bool BeginRender() = 0;

    // Draw an image
    virtual void DrawImage(const Image& image, uint32_t x, uint32_t y) = 0;

    // Draw a sprite
    virtual void DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y) = 0;

    // Complete rendering
    virtual void EndRender() = 0;

    // Get window information
    virtual const WindowInfo& GetWindowInformation() const = 0;

    // Get the key mapping
    virtual KeyMapping& GetKeyMapping() = 0;
};

#ifdef USE_SDL
#include "SDL3/SDL.h"

class SdlBackend : protected Backend
{
  public:
    SdlBackend();
    ~SdlBackend();
    void SetupImage(Image& image);
    void CleanupImage(Image& image);
    bool Update(class InputState& input);
    bool BeginRender();
    void DrawImage(const Image& image, uint32_t x, uint32_t y);
    void DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y);
    void EndRender();
    const WindowInfo& GetWindowInformation() const
    {
        return m_windowInfo;
    }
    KeyMapping& GetKeyMapping()
    {
        return m_mapping;
    }

  private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    WindowInfo m_windowInfo;
    int32_t m_windowId;
    KeyMapping m_mapping;
    float m_scrollAmount;
    SDL_Gamepad* m_gamepad;
    SDL_JoystickID m_gamepadId;

    bool HandleEvent(const SDL_Event& event, InputState& input);

    static constexpr KeyMapping DEFAULT_KEYMAP = {
        SDL_SCANCODE_ESCAPE,
        SDL_SCANCODE_TAB,
        SDL_SCANCODE_Q,
        SDL_SCANCODE_C,
        SDL_SCANCODE_X,
        SDL_SCANCODE_V,
        SDL_SCANCODE_SPACE,
        SDL_SCANCODE_F,
        SDL_SCANCODE_E,
        SDL_SCANCODE_R,
        0,
        0,
        SDL_SCANCODE_LSHIFT,
        SDL_SCANCODE_LCTRL,
        SDL_SCANCODE_W,
        SDL_SCANCODE_S,
        SDL_SCANCODE_A,
        SDL_SCANCODE_D
    };

    static constexpr float SCROLLING_SENSITIVITY = 20.0f;
};
#endif

// Program entry point
extern int GameMain(Backend* backend, std::vector<fs::path> paths);
