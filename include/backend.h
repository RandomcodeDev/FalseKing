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
union KeyMapping {
    struct
    {
        uint32_t esc;        // Start
        uint32_t tab;        // Select
        uint32_t q;          // Up
        uint32_t c;          // Down
        uint32_t x;          // Left
        uint32_t v;          // Right
        uint32_t space;      // A
        uint32_t f;          // B
        uint32_t e;          // X
        uint32_t r;          // Y
        uint32_t scrollUp;   // Left shoulder
        uint32_t scrollDown; // Right shoulder
        uint32_t control;    // Left stick
        uint32_t shift;      // Right stick
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
    virtual void DrawImage(const Image& image, uint32_t x, uint32_t y,
                           float scaleX = 1.0f, float scaleY = 1.0f,
                           uint32_t srcX = 0, uint32_t srcY = 0,
                           uint32_t srcWidth = 0, uint32_t srcHeight = 0,
                           glm::u8vec3 color = glm::u8vec3(255, 255, 255)) = 0;

    // Draw a sprite
    void DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y);

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
    void DrawImage(const Image& image, uint32_t x, uint32_t y, float scaleX,
                   float scaleY, uint32_t srcX, uint32_t srcY,
                   uint32_t srcWidth, uint32_t srcHeight, glm::u8vec3 color);
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
    SDL_Gamepad* m_gamepad;
    SDL_JoystickID m_gamepadId;

    bool HandleEvent(const SDL_Event& event, InputState& input);

    static const inline KeyMapping DEFAULT_KEYMAP = {SDL_SCANCODE_ESCAPE,
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
                                                     SDL_SCANCODE_D};

    static const inline SDL_GamepadButton BUTTONS_IN_ORDER[] = {
        SDL_GAMEPAD_BUTTON_START,
        SDL_GAMEPAD_BUTTON_BACK,
        SDL_GAMEPAD_BUTTON_DPAD_UP,
        SDL_GAMEPAD_BUTTON_DPAD_DOWN,
        SDL_GAMEPAD_BUTTON_DPAD_LEFT,
        SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_A,
        SDL_GAMEPAD_BUTTON_B,
        SDL_GAMEPAD_BUTTON_X,
        SDL_GAMEPAD_BUTTON_Y,
        SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
        SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
        SDL_GAMEPAD_BUTTON_LEFT_STICK,
        SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    };

    static constexpr float SCROLLING_SENSITIVITY = 20.0f;
};
#endif

// Program entry point
extern int GameMain(Backend* backend, std::vector<fs::path> backendPaths);

// Global backend pointer
extern Backend* g_backend;
