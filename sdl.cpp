// SDL backend

#include "SDL3/SDL_main.h"

#include "backend.h"
#include "image.h"
#include "input.h"
#include "sprite.h"

int SDL_main(int argc, char* argv[])
{
#if defined(__WINRT__) && defined(_DEBUG)
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    spdlog::flush_every(std::chrono::seconds(5));
#endif

    Backend* backend = (Backend*)new SdlBackend();

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
    SDL_TriggerBreakpoint();
    exit(exitCode);
}

SdlBackend::SdlBackend()
{
    m_mapping = DEFAULT_KEYMAP;

    if (SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO) < 0)
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

    SDL_SetRelativeMouseMode(SDL_TRUE);

    SPDLOG_INFO("Enumerating gamepads");
    int32_t gamepadCount = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&gamepadCount);
    if (gamepadCount > 0)
    {
        for (int32_t i = 0; i < gamepadCount; i++)
        {
            SDL_Gamepad* gamepad = SDL_OpenGamepad(gamepads[i]);
            if (!gamepad)
            {
                continue;
            }

            SPDLOG_INFO("Gamepad {} player {}: {}", gamepads[i],
                        SDL_GetGamepadPlayerIndex(gamepad),
                        SDL_GetGamepadName(gamepad));
            if (gamepadCount == 1 || SDL_GetGamepadPlayerIndex(gamepad) == 1)
            {
                m_gamepad = gamepad;
                m_gamepadId = gamepads[i];
            }
        }

        SDL_free(gamepads);
    }
    else
    {
        SPDLOG_INFO("No gamepads");
    }
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

bool SdlBackend::Update(InputState& input)
{
    SDL_Event event{};

    // Scrolling is goofy but this seems legit
    if (m_scrollAmount > 0)
    {
        m_scrollAmount--;
    }
    else
    {
        input.state &=
            ~(InputState::LEFT_SHOULDER | InputState::RIGHT_SHOULDER);
    }

    while (SDL_PollEvent(&event))
    {
        if (!HandleEvent(event, input))
        {
            return false;
        }
    }

    return true;
}

bool SdlBackend::HandleEvent(const SDL_Event& event, InputState& input)
{
    if ((event.type >= SDL_EVENT_WINDOW_FIRST &&
         event.type <= SDL_EVENT_WINDOW_LAST) &&
        event.window.windowID == m_windowId)
    {
        switch (event.window.type)
        {
        case SDL_EVENT_WINDOW_FOCUS_GAINED: {
            SPDLOG_INFO("Window focused");
            m_windowInfo.focused = true;
            break;
            ;
        }
        case SDL_EVENT_WINDOW_FOCUS_LOST: {
            SPDLOG_INFO("Window unfocused");
            m_windowInfo.focused = false;
            break;
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
            break;
        }
        }
    }
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
             event.type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        bool down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        if (event.button.button == 1) // left click
        {
            input.rightTrigger = down ? 1.0f : 0.0f;
        }
        else if (event.button.button == 3) // right click
        {
            input.leftTrigger = down ? 1.0f : 0.0f;
        }
    }
    else if (event.type == SDL_EVENT_MOUSE_MOTION)
    {
        // this is a guess
        input.rightStick.x = event.motion.xrel / 5.0f;
        input.rightStick.y = event.motion.yrel / 5.0f;
    }
    else if (event.type == SDL_EVENT_MOUSE_WHEEL)
    {
        input.state |= event.wheel.y > 0 ? InputState::LEFT_SHOULDER
                                         : InputState::RIGHT_SHOULDER;
        m_scrollAmount = abs(event.wheel.y) * SCROLLING_SENSITIVITY;
    }
    else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP)
    {
        SDL_Scancode key = event.key.keysym.scancode;
        bool down = event.type == SDL_EVENT_KEY_DOWN;
        if (key == m_mapping.w)
        {
            input.leftStick.y = down ? -1.0f : 0.0f;
        }
        else if (key == m_mapping.s)
        {
            input.leftStick.y = down ? 1.0f : 0.0f;
        }
        else if (key == m_mapping.a)
        {
            input.leftStick.x = down ? -1.0f : 0.0f;
        }
        else if (key == m_mapping.d)
        {
            input.leftStick.x = down ? 1.0f : 0.0f;
        }
        else
        {
            // There are 4 mappings that aren't bit flags
            for (uint8_t i = 0; i < ARRAY_SIZE(m_mapping.values) - 4; i++)
            {
                if (key == m_mapping.values[i])
                {
                    // Mapping is in same order as bit flags for state
                    // https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
                    uint32_t mapping = 1 << i;
                    input.state = (input.state & ~mapping) | (-down & mapping);
                }
            }
        }
    }
    else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
             event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
    {
        // Same as keyboard buttons
        bool down = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
        for (uint8_t i = 0; i < ARRAY_SIZE(BUTTONS_IN_ORDER); i++)
        {
            if (event.gbutton.button == BUTTONS_IN_ORDER[i])
            {
                uint32_t mapping = 1 << i;
                input.state = (input.state & ~mapping) | (-down & mapping);
            }
        }
    }
    else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION)
    {
        // TODO: make inversion optional (Y axes are backwards
        // from what's interpreted from the keyboard)
        float value = (float)event.gaxis.value / INT16_MAX;
        switch (event.gaxis.axis)
        {
        case SDL_GAMEPAD_AXIS_LEFTX:
            input.leftStick.x = value;
            break;
        case SDL_GAMEPAD_AXIS_LEFTY:
            input.leftStick.y = -value;
            break;
        case SDL_GAMEPAD_AXIS_RIGHTX:
            input.rightStick.x = value;
            break;
        case SDL_GAMEPAD_AXIS_RIGHTY:
            input.rightStick.y = -value;
            break;
        case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
            input.leftTrigger = value;
            break;
        case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
            input.rightTrigger = value;
            break;
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
