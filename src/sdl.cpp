// SDL backend

#ifdef _WIN32
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "backend.h"
#include "image.h"
#include "input.h"
#include "sprite.h"

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
    const std::string& DescribeSystem() const;
    void* LoadLibrary(const std::string& path) const
    {
        std::string fullPath = path +
#ifdef _WIN32
                               ".dll";
#elif defined(__APPLE__)
                               ".dylib";
#else
                               ".so";
#endif
        return SDL_LoadObject(fullPath.c_str());
    }
    Symbol GetSymbol(void* dll, const std::string& symbol) const
    {
        return (Symbol)SDL_LoadFunction(dll, symbol.c_str());
    }
    uint64_t GetFrameCount() const
    {
        return m_frames;
    }
    const std::string& DescribeBackend() const
    {
        return m_description;
    }

  private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    WindowInfo m_windowInfo;
    uint32_t m_windowId;
    KeyMapping m_mapping;
    SDL_Gamepad* m_gamepad;
    SDL_JoystickID m_gamepadId;
    bool m_usingGamepad;
    uint64_t m_frames;
    std::string m_description;

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

extern "C" int main(int argc, char* argv[])
{
#ifdef _WIN32
#ifdef _DEBUG
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    spdlog::flush_every(std::chrono::seconds(5));
#endif

    spdlog::default_logger().get()->sinks().push_back(
        std::make_shared<spdlog::sinks::msvc_sink_st>());
#endif

    spdlog::default_logger().get()->sinks().push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_st>(fmt::format(
            "{}FalseKing-{:%Y-%m-%d_%H-%M-%S}.log",
            SDL_GetPrefPath("", GAME_NAME), chrono::system_clock::now())));

    SPDLOG_INFO("Creating backend");
    Backend* backend = (Backend*)new SdlBackend();

    std::vector<std::string> paths;
    std::string baseDir = SDL_GetBasePath();
#ifdef __APPLE__
    baseDir += "Contents/Resources/";
#endif
#ifndef __WINRT__
    baseDir += "assets";
#endif
    paths.push_back(baseDir);
    int returnCode = GameMain(backend, paths);
    SPDLOG_INFO("Destroying backend");
    delete (SdlBackend*)backend;
    return returnCode;
}

[[noreturn]] void Quit(const std::string& message, int32_t exitCode)
{
    std::string title = fmt::format("Error {0}/0x{0:X}", exitCode);
    SDL_ShowSimpleMessageBox(0, title.c_str(), message.c_str(), nullptr);
    SDL_TriggerBreakpoint();
    exit(exitCode);
}

SdlBackend::SdlBackend()
{
    m_mapping = DEFAULT_KEYMAP;

    if (SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO) < 0)
    {
        QUIT("Failed to initialize SDL: {}", SDL_GetError());
    }

    m_window =
        SDL_CreateWindow(GAME_NAME, 1024, 576,
                         SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        QUIT("Failed to create window: {}", SDL_GetError());
    }

    m_renderer = SDL_CreateRenderer(m_window,
#if defined(__APPLE__)
                                    "metal",
#elif _WIN32_WINNT == _WIN32_WINNT_WINXP
                                    "opengl",
#elif defined(_WIN32) && !defined(__WINRT__)
                                    "direct3d12",
#else
                                    nullptr,
#endif
                                    0);
    if (!m_renderer)
    {
        QUIT("Failed to create renderer: {}", SDL_GetError());
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    m_windowId = SDL_GetWindowID(m_window);
    m_windowInfo.handle = m_window;
    SDL_GetWindowSize(m_window, &m_windowInfo.width, &m_windowInfo.height);
    m_windowInfo.focused = true;

    // SDL_SetRelativeMouseMode(SDL_TRUE);

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

    SDL_version version;
    SDL_GetVersion(&version);
    SDL_RendererInfo rendererInfo;
    SDL_GetRendererInfo(m_renderer, &rendererInfo);
    m_description = fmt::format("SDL v{}.{}.{} video {} render {}",
                                version.major, version.minor, version.patch,
                                SDL_GetCurrentVideoDriver(), rendererInfo.name);

    m_frames = 0;
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
        QUIT("Failed to create texture for image: {}", SDL_GetError());
    }

    SDL_SetTextureScaleMode((SDL_Texture*)image.backendData,
                            SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode((SDL_Texture*)image.backendData,
                            SDL_BLENDMODE_BLEND);

    if (SDL_UpdateTexture((SDL_Texture*)image.backendData, nullptr,
                          image.GetPixels(),
                          image.GetChannels() * imageWidth) < 0)
    {
        QUIT("Failed to copy pixels to texture for image: {}", SDL_GetError());
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

        if (!m_usingGamepad)
        {
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

            // There are 4 mappings that aren't bit flags, they're handled above
            for (uint8_t i = 0; i < ARRAY_SIZE(m_mapping.values) - 4; i++)
            {
                bool down = keys[m_mapping.values[i]];
                // Mapping is in same order as bit flags for state
                // https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
                uint32_t mapping = 1 << i;
                input.state = (input.state & ~mapping) | (-down & mapping);
            }
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
        m_usingGamepad = false;
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
        m_usingGamepad = false;
        // this is a guess
        input.rightStick.x = event.motion.xrel / 5.0f;
        input.rightStick.y = event.motion.yrel / 5.0f;
    }
    else if (event.type == SDL_EVENT_MOUSE_WHEEL)
    {
        m_usingGamepad = false;
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
        m_usingGamepad = true;
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
        m_usingGamepad = true;
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
    m_frames++;
}

const std::string& SdlBackend::DescribeSystem() const
{
    static std::string Description;

    // This shouldn't ever change between runs
    if (Description.length() > 0)
    {
        return Description;
    }

#ifdef _WIN32
    // All versions
    HKEY CurrentVersionHandle;
    CHAR EditionId[32] = {};
    CHAR ProductName[32] = {};
    DWORD Size;

    // Windows 10 and above
    CHAR InstallationType[32] = {};
    DWORD CurrentMajorVersionNumber;
    DWORD CurrentMinorVersionNumber;
    CHAR CurrentBuildNumber[8] = {};
    DWORD UBR;
    CHAR DisplayVersion[8] = {};
    CHAR BuildLabEx[64] = {};

    // Windows 8 and below
    CHAR CurrentVersion[4] = {};
    CHAR CSDVersion[8] = {};
    CHAR BuildLab[64] = {};

    PCSTR Name = nullptr;

    RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                  "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0,
                  KEY_QUERY_VALUE, &CurrentVersionHandle);

    Size = sizeof(EditionId);
    RegQueryValueExA(CurrentVersionHandle, "EditionID", nullptr, nullptr,
                     (LPBYTE)EditionId, &Size);

    Size = sizeof(ProductName);
    RegQueryValueExA(CurrentVersionHandle, "ProductName", nullptr, nullptr,
                     (LPBYTE)ProductName, &Size);

    Size = sizeof(INT);
    if (RegQueryValueExA(CurrentVersionHandle, "CurrentMajorVersionNumber",
                         nullptr, nullptr, (LPBYTE)&CurrentMajorVersionNumber,
                         &Size) == ERROR_SUCCESS)
    {
        Size = sizeof(InstallationType);
        RegQueryValueExA(CurrentVersionHandle, "InstallationType", nullptr,
                         nullptr, (LPBYTE)InstallationType, &Size);

        Size = sizeof(INT);
        RegQueryValueExA(CurrentVersionHandle, "CurrentMinorVersionNumber",
                         nullptr, nullptr, (LPBYTE)&CurrentMinorVersionNumber,
                         &Size);

        Size = sizeof(CurrentBuildNumber);
        RegQueryValueExA(CurrentVersionHandle, "CurrentBuildNumber", nullptr,
                         nullptr, (LPBYTE)CurrentBuildNumber, &Size);

        Size = sizeof(INT);
        RegQueryValueExA(CurrentVersionHandle, "UBR", nullptr, nullptr,
                         (LPBYTE)&UBR, &Size);

        Size = sizeof(BuildLabEx);
        RegQueryValueExA(CurrentVersionHandle, "BuildLabEx", nullptr, nullptr,
                         (LPBYTE)&BuildLabEx, &Size);

        Size = sizeof(DisplayVersion);
        RegQueryValueExA(CurrentVersionHandle, "DisplayVersion", nullptr,
                         nullptr, (LPBYTE)DisplayVersion, &Size);

        std::string edition(EditionId,
                            std::min(strlen(EditionId), ARRAY_SIZE(EditionId)));
        Description = fmt::format(
#ifdef _DEBUG
            "{} {} {}.{}.{}.{} {} (build lab {})",
#else
            "{} {} {}.{}.{}.{} {}",
#endif
            edition == "SystemOS" ? "Xbox System Software" : "Windows",
            (strncmp(InstallationType, "Client",
                     ARRAY_SIZE(InstallationType)) == 0)
                ? "Desktop"
                : InstallationType,
            CurrentMajorVersionNumber, CurrentMinorVersionNumber,
            CurrentBuildNumber, UBR, EditionId
#ifdef _DEBUG
            ,
            BuildLabEx, ProductName, DisplayVersion
#endif
        );
    }
    else
    {
        Size = sizeof(CSDVersion);
        RegQueryValueExA(CurrentVersionHandle, "CSDVersion", nullptr, nullptr,
                         (LPBYTE)BuildLab, &Size);
        Size = sizeof(BuildLab);
        RegQueryValueExA(CurrentVersionHandle, "BuildLab", nullptr, nullptr,
                         (LPBYTE)BuildLab, &Size);

        Description = fmt::format("Windows {} {} {} (build lab {})",
                                  ProductName, EditionId, CSDVersion, BuildLab);
    }
#elif defined __APPLE__
    std::string osVersion;
    osVersion.resize(32);
    size_t osVersionSize = osVersion.size() - 1;
    int32_t osVersionNames[] = {CTL_KERN, KERN_OSRELEASE};

    if (sysctl(osVersionNames, 2, osVersion.data(), &osVersionSize, nullptr,
               0) == -1)
    {
        Description =
            fmt::format("macOS <unknown version: {}>", strerror(errno));
    }
    else
    {
        osVersion.resize(osVersionSize - 1);

        uint16_t major;
        uint16_t minor;

        std::unordered_map<uint16_t, std::string> versionNames;
        versionNames[0x0E00] = "Sonoma";
        versionNames[0x0D00] = "Ventura";
        versionNames[0x0C00] = "Monterey";
        versionNames[0x0B00] = "Big Sur";
        versionNames[0x0A0F] = "Catalina";
        versionNames[0x0A0E] = "Mojave";
        versionNames[0x0A0D] = "High Sierra";

        std::istringstream versionStream(osVersion);
        versionStream >> major;
        versionStream.get();
        versionStream >> minor;

        if (major >= 20)
        {
            major -= 9;
            minor -= 1;
            Description = fmt::format("macOS {}.{} {}", major, minor,
                                      versionNames[major << 8]);
        }
        else
        {
            major -= 4;
            minor += 1;
            Description = fmt::format("macOS 10.{}.{} {}", major, minor,
                                      versionNames[0x0A00 | major]);
        }
    }
#else
    struct utsname utsName = {};
    uname(&utsName);

    std::fstream osRelease("/etc/os-release", std::ios::ate);
    if (osRelease.good())
    {
        std::string osReleaseContent;
        osReleaseContent.resize(osRelease.tellg());
        osRelease.seekg(std::ios::beg);
        osRelease.read(osReleaseContent.data(), osReleaseContent.size());
        osRelease.close();
        size_t nameOff = osReleaseContent.find("PRETTY_NAME=\"");
        std::string name;
        size_t nameEndOff = 0;
        if (nameOff == SIZE_MAX)
        {
            name = "Unknown";
        }
        else
        {
            nameOff += 13; // PRETTY_NAME="
            nameEndOff = std::string_view(osReleaseContent.data() + nameOff,
                                          osReleaseContent.size() - nameOff)
                             .find('\"');
            name.assign(osReleaseContent.data(), nameOff, nameEndOff);
        }

        size_t buildIdOff =
            std::string_view(osReleaseContent.data() + nameEndOff + 1,
                             osReleaseContent.size() - nameEndOff - 1)
                .find("BUILD_ID=");
        std::string buildId;
        if (buildIdOff == SIZE_MAX)
        {
            buildId = "unknown";
        }
        else
        {
            buildIdOff += 9; // BUILD_ID=
            size_t buildIdEndOff =
                std::string_view(osReleaseContent.data() + buildIdOff,
                                 osReleaseContent.size() - buildIdOff)
                    .find('\n');
            buildId.assign(osReleaseContent.data(), buildIdOff, buildIdEndOff);
        }
        Description = fmt::format("{} {}, kernel {} {} {}, host {}", name,
                                  buildId, utsName.sysname, utsName.release,
                                  utsName.machine, utsName.nodename);
    }
    else
    {
        Description =
            fmt::format("%s %s %s, host %s", utsName.sysname, utsName.release,
                        utsName.machine, utsName.nodename);
    }
#endif
    return Description;
}
