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

    // Get a string describing the system
    virtual const std::string& DescribeSystem() const = 0;
};

#if __NX__
class SwitchBackend;
#elif defined(USE_SDL)
class SdlBackend;
#endif

// Program entry point
extern int GameMain(Backend* backend, std::vector<std::string> backendPaths);

// Global backend pointer
extern Backend* g_backend;
