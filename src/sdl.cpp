// SDL backend

#include "SDL3/SDL_main.h"

#include "backend.h"
#include "image.h"
#include "input.h"
#include "sprite.h"

extern "C" int main(int argc, char* argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    spdlog::flush_every(std::chrono::seconds(5));
#endif

    SPDLOG_INFO("Creating backend");
    Backend* backend = (Backend*)new SdlBackend();

    std::vector<fs::path> paths;
    paths.push_back(SDL_GetBasePath());
    int returnCode = GameMain(backend, paths);
    SPDLOG_INFO("Destroying backend");
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

    while (SDL_PollEvent(&event))
    {
        if (!HandleEvent(event, input))
        {
            return false;
        }

        int32_t keyCount = 0;
        SDL_GetKeyboardState(&keyCount);
        bool* keys = new bool[keyCount];
        keys = (bool*)SDL_GetKeyboardState(&keyCount);

        bool w = keys[m_mapping.w];
        bool s = keys[m_mapping.s];
        bool a = keys[m_mapping.a];
        bool d = keys[m_mapping.d];

        if (w && !s)
        {
            input.leftStick.y = -1.0f;
        }
        else if (!w && s)
        {
            input.leftStick.y = 1.0f;
        }
        else
        {
            input.leftStick.y = 0.0f;
        }

        if (a && !d)
        {
            input.leftStick.x = -1.0f;
        }
        else if (!a && d)
        {
            input.leftStick.x = 1.0f;
        }
        else
        {
            input.leftStick.x = 0.0f;
        }

        // There are 4 mappings that aren't bit flags
        for (uint8_t i = 0; i < ARRAY_SIZE(m_mapping.values) - 4; i++)
        {
            bool down = keys[m_mapping.values[i]];
            // Mapping is in same order as bit flags for state
            // https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
            uint32_t mapping = 1 << i;
            input.state = (input.state & ~mapping) | (-down & mapping);
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
        uint16_t mask = event.wheel.y > 0 ? InputState::LEFT_SHOULDER
                                          : InputState::RIGHT_SHOULDER;
        input.state &= event.wheel.y > 0 ? ~InputState::LEFT_SHOULDER
                                         : ~InputState::RIGHT_SHOULDER;
        input.state |= mask;
        input.scrollAmount = event.wheel.y / 19;
    }
    else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
             event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
    {
        // Same as keyboard buttons
        bool down = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
        if (event.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)
        {
            input.state = (input.state & ~InputState::RIGHT_SHOULDER) |
                          (-down & InputState::RIGHT_SHOULDER);
            input.scrollAmount = down ? -1.0f : 0.0f;
        }
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)
        {
            input.state = (input.state & ~InputState::LEFT_SHOULDER) |
                          (-down & InputState::LEFT_SHOULDER);
            input.scrollAmount = down ? -1.0f : 0.0f;
        }
        else
        {
            for (uint8_t i = 0; i < ARRAY_SIZE(BUTTONS_IN_ORDER); i++)
            {
                if (event.gbutton.button == BUTTONS_IN_ORDER[i])
                {
                    uint32_t mapping = 1 << i;
                    input.state = (input.state & ~mapping) | (-down & mapping);
                }
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
            input.leftStick.y = value;
            break;
        case SDL_GAMEPAD_AXIS_RIGHTX:
            input.rightStick.x = value;
            break;
        case SDL_GAMEPAD_AXIS_RIGHTY:
            input.rightStick.y = value;
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

    SDL_SetRenderDrawColor(m_renderer, 128, 128, 128, 255);
    SDL_RenderClear(m_renderer);
    return true;
}

void SdlBackend::DrawImage(const Image& image, uint32_t x, uint32_t y,
                           float scaleX, float scaleY, uint32_t srcX,
                           uint32_t srcY, uint32_t srcWidth, uint32_t srcHeight,
                           glm::u8vec3 color)
{
    SDL_SetRenderTarget(m_renderer, nullptr);
    uint32_t imageWidth;
    uint32_t imageHeight;
    image.GetSize(imageWidth, imageHeight);

    if (srcWidth > 0)
    {
        imageWidth = srcWidth;
    }
    if (srcHeight > 0)
    {
        imageHeight = srcHeight;
    }
    SDL_FRect srcRegion{(float)srcX, (float)srcY, (float)imageWidth,
                        (float)imageHeight};
    imageWidth = (uint32_t)(imageWidth * scaleX);
    imageHeight = (uint32_t)(imageHeight * scaleY);
    SDL_FRect destRegion{(float)x, (float)y, (float)imageWidth,
                         (float)imageHeight};

    float xScale = 0;
    float yScale = 0;
    SDL_GetRenderScale(m_renderer, &xScale, &yScale);
    SDL_SetRenderScale(m_renderer, xScale + scaleX, yScale + scaleY);
    SDL_SetTextureColorMod((SDL_Texture*)image.backendData, color.r, color.g,
                           color.b);
    SDL_RenderTexture(m_renderer, (SDL_Texture*)image.backendData, &srcRegion,
                      &destRegion);
    SDL_SetTextureColorMod((SDL_Texture*)image.backendData, 255, 255, 255);
    SDL_SetRenderScale(m_renderer, xScale, yScale);
    SDL_SetRenderTarget(m_renderer, nullptr);
}

void SdlBackend::EndRender()
{
    SDL_RenderPresent(m_renderer);
}