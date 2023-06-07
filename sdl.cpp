// SDL backend

#include "SDL3/SDL_main.h"

#include "backend.h"
#include "image.h"
#include "sprite.h"

int SDL_main(int argc, char* argv[])
{
    Backend* backend = (Backend*)new SdlBackend();
    int returnCode = GameMain(backend);
    delete backend;
    return returnCode;
}

[[noreturn]]
void Quit(const std::string& message, int32_t exitCode)
{
    std::string title = fmt::format("Error {0}/0x{0:X}", exitCode);
    SDL_ShowSimpleMessageBox(0, title.c_str(), message.c_str(), NULL);
    exit(exitCode);
}

SdlBackend::SdlBackend()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        Quit(fmt::format("Failed to initialize SDL: {}", SDL_GetError()), 1);
    }

    m_renderer = nullptr;
    m_window = nullptr;
    if (SDL_CreateWindowAndRenderer(1024, 576, SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE, &m_window, &m_renderer) < 0)
    {
        Quit(fmt::format("Failed to create window or renderer: {}", SDL_GetError()), 1);
    }
    
    m_windowId = SDL_GetWindowID(m_window);
    m_windowInfo.handle = m_window;
    SDL_GetWindowSize(m_window, &m_windowInfo.width, &m_windowInfo.height);
    m_windowInfo.focused = true;
}

SdlBackend::~SdlBackend()
{
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void SdlBackend::SetupImage(Image& image)
{
    uint32_t imageWidth;
    uint32_t imageHeight;
    image.GetSize(imageWidth, imageHeight);
    image.backendData = SDL_CreateSurfaceFrom(image.GetPixels(), imageWidth, imageHeight, 4, SDL_PIXELFORMAT_RGBA8888);
    if (!image.backendData)
    {
        Quit(fmt::format("Failed to create surface from image: {}", SDL_GetError()), 1);
    }
}

void SdlBackend::CleanupImage(Image& image)
{
    if (image.backendData)
    {
        SDL_DestroySurface((SDL_Surface*)image.backendData);
    }
    image.backendData = nullptr;
}

void SdlBackend::SetupSprite(const Image& spriteSheet, Sprite& sprite)
{
    if (!spriteSheet.backendData)
    {
        sprite.backendData = nullptr;
        return;
    }


}

void SdlBackend::CleanupSprite(Sprite& sprite)
{
    sprite.backendData = nullptr;
}

bool SdlBackend::Update()
{
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
        if (!HandleEvent(event))
        {
            return false;
        }
    }

    return true;
}

bool SdlBackend::HandleEvent(const SDL_Event& event)
{
    if (event.type >= SDL_EVENT_WINDOW_FIRST ||
        event.type <= SDL_EVENT_WINDOW_LAST &&
        event.window.windowID == m_windowId)
    {
        switch (event.window.type)
        {
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        {
            SPDLOG_INFO("Window focused");
            m_windowInfo.focused = true;
            return true;
        }
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            SPDLOG_INFO("Window unfocused");
            m_windowInfo.focused = false;
            return true;
        }
        case SDL_EVENT_WINDOW_RESIZED:
        {
            SPDLOG_INFO("Window resized from {}x{} to {}x{}", m_windowInfo.width,
                m_windowInfo.height, event.window.data1, event.window.data2);

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

bool SdlBackend::BeginRender()
{
    if (!m_windowInfo.focused)
    {
        return false;
    }

    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_renderer);
    return true;
}

void SdlBackend::EndRender()
{
    SDL_RenderPresent(m_renderer);
}
