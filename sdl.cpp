// SDL backend

#include "SDL3/SDL_main.h"

#include "backend.h"
#include "image.h"
#include "sprite.h"

int SDL_main(int argc, char* argv[])
{
    Backend* backend = (Backend*)new SdlBackend();

#if defined(__WINRT__) && defined(_DEBUG)
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    spdlog::flush_every(std::chrono::seconds(5));
#endif

    std::vector<fs::path> paths;
    paths.push_back(SDL_GetBasePath());
    int returnCode = GameMain(backend, paths);
    delete backend;
    return returnCode;
}

[[noreturn]] void Quit(const std::string& message, int32_t exitCode)
{
    std::string title = fmt::format("Error {0}/0x{0:X}", exitCode);
    SDL_ShowSimpleMessageBox(0, title.c_str(), message.c_str(), NULL);
    exit(exitCode);
}

SdlBackend::SdlBackend()
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0)
    {
        Quit(fmt::format("Failed to initialize SDL: {}", SDL_GetError()));
    }

    m_window =
        SDL_CreateWindow(GAME_NAME, 1024, 576,
                         SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        Quit(fmt::format("Failed to create window: {}", SDL_GetError()));
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr, 0);
    if (!m_renderer)
    {
        Quit(fmt::format("Failed to create renderer: {}", SDL_GetError()));
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

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
    image.backendData =
        SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ABGR8888,
                          SDL_TEXTUREACCESS_TARGET, imageWidth, imageHeight);
    if (!image.backendData)
    {
        Quit(fmt::format("Failed to create texture for image: {}",
                         SDL_GetError()),
             1);
    }

    SDL_SetTextureScaleMode((SDL_Texture*)image.backendData,
                            SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode((SDL_Texture*)image.backendData,
                            SDL_BLENDMODE_BLEND);

    if (SDL_UpdateTexture((SDL_Texture*)image.backendData, nullptr,
                          image.GetPixels(),
                          image.GetChannels() * imageWidth) < 0)
    {
        Quit(fmt::format("Failed to copy pixels to texture for image: {}",
                         SDL_GetError()),
             1);
    }
}

void SdlBackend::CleanupImage(Image& image)
{
    if (image.backendData)
    {
        SDL_DestroyTexture((SDL_Texture*)image.backendData);
    }
    image.backendData = nullptr;
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
        case SDL_EVENT_WINDOW_FOCUS_GAINED: {
            SPDLOG_INFO("Window focused");
            m_windowInfo.focused = true;
            return true;
        }
        case SDL_EVENT_WINDOW_FOCUS_LOST: {
            SPDLOG_INFO("Window unfocused");
            m_windowInfo.focused = false;
            return true;
        }
        case SDL_EVENT_WINDOW_RESIZED: {
            if (m_windowInfo.width != event.window.data1 ||
                m_windowInfo.height != event.window.data2)
            {
                SPDLOG_INFO("Window resized from {}x{} to {}x{}",
                            m_windowInfo.width, m_windowInfo.height,
                            event.window.data1, event.window.data2);
                m_windowInfo.width = event.window.data1;
                m_windowInfo.height = event.window.data2;
            }

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

    // Scale everything to be the right size
    SDL_SetRenderScale(m_renderer, (float)m_windowInfo.width / GAME_WIDTH,
                       (float)m_windowInfo.height / GAME_HEIGHT);

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_renderer);
    return true;
}

void SdlBackend::DrawImage(const Image& image, uint32_t x, uint32_t y)
{
    SDL_SetRenderTarget(m_renderer, nullptr);
    uint32_t imageWidth;
    uint32_t imageHeight;
    image.GetSize(imageWidth, imageHeight);
    SDL_FRect region{(float)x, (float)y, (float)imageWidth, (float)imageHeight};
    SDL_RenderTexture(m_renderer, (SDL_Texture*)image.backendData, nullptr,
                      &region);
}

void SdlBackend::DrawSprite(const Sprite& sprite, uint32_t x, uint32_t y)
{
    SDL_SetRenderTarget(m_renderer, nullptr);
    SDL_FRect srcRegion{(float)sprite.x, (float)sprite.y, (float)sprite.width,
                        (float)sprite.height};
    SDL_FRect destRegion{(float)x, (float)y, (float)sprite.width,
                         (float)sprite.height};
    SDL_RenderTexture(m_renderer, (SDL_Texture*)sprite.sheet.backendData,
                      &srcRegion, &destRegion);
}

void SdlBackend::EndRender()
{
    SDL_RenderPresent(m_renderer);
}
