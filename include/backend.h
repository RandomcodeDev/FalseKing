#pragma once

#include "game.h"

// Window information
struct WindowInfo
{
    void* handle;
    int32_t width;
    int32_t height;
    int32_t lastWidth;
    int32_t lastHeight;
    bool focused;
    bool resized;
};

// Map of backend keycodes to controller inputs
union KeyMapping {
    struct
    {
        uint32_t start;
        uint32_t select;
        uint32_t up;
        uint32_t down;
        uint32_t left;
        uint32_t right;
        uint32_t a;
        uint32_t b;
        uint32_t x;
        uint32_t y;
        uint32_t leftShoulder;
        uint32_t rightShoulder;
        uint32_t leftPress;
        uint32_t rightPress;
        uint32_t leftUp;
        uint32_t leftDown;
        uint32_t leftLeft;
        uint32_t leftRight;
    };
    uint32_t values[18];
};

// Forward declarations
class Image;
struct Sprite;
namespace Input
{
class State;
}

// Abstraction of the platform
class Backend
{
  public:
    // Set up an image so it can be used
    virtual void SetupImage(Image& image) = 0;

    // Clean up an image's resources
    virtual void CleanupImage(Image& image) = 0;

    // Process events
    virtual bool Update(class Input::State& input) = 0;

    // Set up ImGui backend
    virtual void InitializeImGui() = 0;

    // Clean up ImGui backend
    virtual void ShutdownImGui() = 0;

    // Prepare for rendering
    virtual void BeginRender() = 0;

    // Draw an image
    virtual void DrawImage(const Image& image, uint32_t x, uint32_t y,
                           float scaleX = 1.0f, float scaleY = 1.0f,
                           uint32_t srcX = 0, uint32_t srcY = 0,
                           uint32_t srcWidth = 0, uint32_t srcHeight = 0,
                           PxVec3 color = PxVec3(1, 1, 1)) = 0;

    // Draw a sprite
    void DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y, bool center = true);

    // Complete rendering
    virtual void EndRender() = 0;

    // Get the number of frames rendered
    virtual uint64_t GetFrameCount() const = 0;

    // Get window information
    virtual const WindowInfo& GetWindowInformation() const = 0;

    // Get the key mapping
    virtual KeyMapping& GetKeyMapping() = 0;

    // Get a string describing the system
    virtual const std::string& DescribeSystem() const = 0;

    // Get information about the backend
    virtual const std::string& DescribeBackend() const = 0;

    // Load a library
    virtual void* LoadLibrary(const std::string& path) const = 0;

    // A dummy function pointer, should be cast to the right signature
    typedef void* (*Symbol)(...);

    // Get a symbol from a library
    virtual Symbol GetSymbol(void* dll, const std::string& path) const = 0;
};

#ifdef __NX__
class SwitchBackend;
#elif defined(USE_SDL)
class SdlBackend;
#endif

// Program entry point
extern int32_t GameMain(Backend* backend, std::vector<std::string> backendPaths);

// Global backend pointer
extern Backend* g_backend;
