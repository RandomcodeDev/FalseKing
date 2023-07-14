#include <nn/fs.h>
#include <nn/hid.h>
#include <nn/nn_Log.h>
#include <nn/oe.h>
#include <nn/oe/oe_DebugApis.h>
#include <nn/vi.h>

#include <nv/nv_MemoryManagement.h>

#include <nvn/nvn.h>
#include <nvn/nvn_Cpp.h>
#include <nvn/nvn_CppFuncPtrImpl.h>
#include <nvn/nvn_CppMethods.h>

#include "backend.h"
#include "fs.h"
#include "image.h"
#include "input.h"
#include "sprite.h"

// template <typename Mutex>
// class SwitchSink : public spdlog::sinks::base_sink<Mutex>
//{
//   public:
//     SwitchSink() {}
//
//   protected:
//     void sink_it_(const spdlog::details::log_msg& msg) override
//     {
//         spdlog::memory_buf_t formatted;
//         spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
//         formatted.push_back(
//             '\0'); // add a null terminator for NN_LOG
//         NN_PUT(formatted.begin(), formatted.size());
//     }
//
//     void flush_() override
//     {
//     }
// };

class SwitchBackend : protected Backend
{
  public:
    SwitchBackend();
    ~SwitchBackend();
    void SetupImage(Image& image);
    void CleanupImage(Image& image);
    bool Update(class Input::State& input);
    bool BeginRender();
    void DrawImage(const Image& image, uint32_t x, uint32_t y, float scaleX,
                   float scaleY, uint32_t srcX, uint32_t srcY,
                   uint32_t srcWidth, uint32_t srcHeight, glm::u8vec3 color);
    void EndRender();
    const WindowInfo& GetWindowInformation() const
    {
        return m_windowInfo;
    }
    // Unused, no keyboard support on Switch
    KeyMapping& GetKeyMapping()
    {
        return m_mapping;
    }
    const std::string& DescribeSystem() const;

    // No actual use for these on Switch yet
    void* LoadLibrary(const std::string& path) const
    {
        return nullptr;
    }
    Symbol GetSymbol(void* dll, const std::string& symbol) const
    {
        return nullptr;
    }
    uint64_t GetFrameCount() const
    {
        return m_frameCount;
    }
    const std::string& DescribeBackend() const
    {
        return m_description;
    }

    static const inline char* ROM_MOUNT = "rom";

  private:
    WindowInfo m_windowInfo;

    KeyMapping m_mapping; // unused

    void* m_fsRomCache;
    size_t m_fsRomCacheSize;

    nn::vi::Display* m_display;
    nn::vi::Layer* m_layer;

    nvn::Device m_device;

    uint64_t m_frameCount;

    std::string m_description;

    // Graphics memory stuff

    // Initialize graphics stuff
    void InitializeGraphics();

    // Clean up graphics stuff
    void ShutdownGraphics();

    // Allocation functions
    static void* GraphicsAllocate(size_t size, size_t alignment,
                                  void* userData);
    static void GraphicsFree(void* memory, void* userData);
    static void* GraphicsReallocate(void* oldMemory, size_t newSize,
                                    void* userData);

    // NVN debug callback
    static void NVNAPIENTRY NvnDebug(nvn::DebugCallbackSource::Enum source,
                                     nvn::DebugCallbackType::Enum type, int id,
                                     nvn::DebugCallbackSeverity::Enum severity,
                                     const char* message,
                                     void* userParams) NN_NOEXCEPT;
};

extern "C" int nnMain()
{
    nn::oe::Initialize();

    // stdout is already sent to the log
    // spdlog::default_logger().get()->sinks().push_back(
    //    std::make_shared<SwitchSink<spdlog::details::null_mutex>>());

    SPDLOG_INFO("Creating backend");
    Backend* backend = (Backend*)new SwitchBackend();

    std::vector<std::string> paths;
    std::string mountPoint(SwitchBackend::ROM_MOUNT);
    mountPoint += ":";
    paths.push_back(mountPoint);
    int returnCode = GameMain(backend, paths);
    SPDLOG_INFO("Destroying backend");
    delete (SwitchBackend*)backend;
    return returnCode;
}

[[noreturn]] void Quit(const std::string& message, int32_t exitCode)
{
    NN_ABORT(message.c_str());
}

SwitchBackend::SwitchBackend() : m_mapping{}
{
    SPDLOG_INFO("Initializing Switch backend");

    nn::fs::QueryMountRomCacheSize(&m_fsRomCacheSize);
    SPDLOG_INFO("Mounting RomFS with {}-byte cache to {}", m_fsRomCacheSize,
                ROM_MOUNT);
    m_fsRomCache = new uint8_t[m_fsRomCacheSize];
    if (!m_fsRomCache)
    {
        Quit("Failed to allocate RomFS cache");
    }
    nn::Result result =
        nn::fs::MountRom(ROM_MOUNT, m_fsRomCache, m_fsRomCacheSize);
    if (result.IsFailure())
    {
        QUIT("Failed to mount RomFS: {}", result.GetDescription());
    }

    nn::vi::Initialize();

    SPDLOG_INFO("Opening display");
    result = nn::vi::OpenDefaultDisplay(&m_display);
    if (result.IsFailure())
    {
        QUIT("Failed to open display: {}", result.GetDescription());
    }

    m_windowInfo.focused = true;
    m_windowInfo.width = 1280;
    m_windowInfo.height = 720;
    SPDLOG_INFO("Creating {}x{} layer", m_windowInfo.width,
                m_windowInfo.height);
    result = nn::vi::CreateLayer(&m_layer, m_display);
    if (result.IsFailure())
    {
        QUIT("Failed to create layer: {}", result.GetDescription());
    }

    SPDLOG_INFO("Initializing input");
    nn::hid::InitializeNpad();

    m_windowInfo.handle = m_layer;

    InitializeGraphics();

    SPDLOG_INFO("Switch backend initialized");
}

SwitchBackend::~SwitchBackend()
{
    SPDLOG_INFO("Shutting down Switch backend");

    ShutdownGraphics();
    nn::vi::DestroyLayer(m_layer);
    nn::vi::CloseDisplay(m_display);
    nn::vi::Finalize();
    nn::fs::Unmount(ROM_MOUNT);
    delete[] (uint8_t*)m_fsRomCache;

    SPDLOG_INFO("Switch backend shut down");
}

static NN_ALIGNAS(0x1000) uint8_t s_graphicsFirmwareMemory[8 * 1024 * 1024];

extern "C" nvn::GenericFuncPtrFunc NVNAPIENTRY
nvnBootstrapLoader(const char* name) NN_NOEXCEPT;

// Recompiling PhysX is annoying
extern "C" int pthread_cancel(pthread_t unused)
{
    return 0;
}

void SwitchBackend::InitializeGraphics()
{
    SPDLOG_INFO("Initializing graphics");

    nv::SetGraphicsAllocator(GraphicsAllocate, GraphicsFree, GraphicsReallocate,
                             nullptr);
    nv::SetGraphicsDevtoolsAllocator(GraphicsAllocate, GraphicsFree,
                                     GraphicsReallocate, nullptr);
    nv::InitializeGraphics(s_graphicsFirmwareMemory,
                           sizeof(s_graphicsFirmwareMemory));

    nn::vi::NativeWindowHandle windowHandle = nullptr;
    nn::vi::GetNativeWindow(&windowHandle, m_layer);

    SPDLOG_INFO("Loading NVN");
    nvn::DeviceGetProcAddressFunc nvnDeviceGetProcAddr =
        (nvn::DeviceGetProcAddressFunc)nvnBootstrapLoader(
            "nvnDeviceGetProcAddress");
    nvn::nvnLoadCPPProcs(nullptr, nvnDeviceGetProcAddr);
    SPDLOG_INFO("NVN loaded");

    SPDLOG_INFO("Creating device");
    nvn::DeviceBuilder deviceBuilder;
    deviceBuilder.SetDefaults();
#ifdef _DEBUG
    deviceBuilder.SetFlags(NVN_DEVICE_FLAG_DEBUG_ENABLE_LEVEL_2_BIT |
                           NVN_DEVICE_FLAG_DEBUG_SKIP_CALLS_ON_ERROR_BIT);
#endif

    //if (!m_device.Initialize(&deviceBuilder))
    //{
        //QUIT("Failed to create NVN device");
    //}
    SPDLOG_INFO("Device created");

#ifdef _DEBUG
    SPDLOG_INFO("Setting debug callback");
    m_device.InstallDebugCallback(NvnDebug, nullptr, true);
#endif

    SPDLOG_INFO("Loading remaining NVN functions");
    nvn::nvnLoadCPPProcs(&m_device, nvnDeviceGetProcAddr);

    SPDLOG_INFO("Graphics initialized");
}

void SwitchBackend::ShutdownGraphics()
{
    SPDLOG_INFO("Shutting down graphics");

    nv::FinalizeGraphics();

    SPDLOG_INFO("Graphics shut down");
}

void* SwitchBackend::GraphicsAllocate(size_t size, size_t alignment,
                                      void* userData)
{
    SPDLOG_TRACE("{}-byte graphics allocation aligned to {}", size, alignment);
    return aligned_alloc(size, alignment);
}

void SwitchBackend::GraphicsFree(void* memory, void* userData)
{
    SPDLOG_TRACE("Free graphics allocation 0x{}", memory);
    free(memory);
}

void* SwitchBackend::GraphicsReallocate(void* oldMemory, size_t newSize,
                                        void* userData)
{
    SPDLOG_TRACE("Reallocate graphics allocation 0x{} to {} bytes", oldMemory,
                 newSize);
    return realloc(oldMemory, newSize);
}

static std::unordered_map<nvn::DebugCallbackSeverity::Enum,
                          spdlog::level::level_enum>
    s_levelMap;

void NVNAPIENTRY SwitchBackend::NvnDebug(
    nvn::DebugCallbackSource::Enum source, nvn::DebugCallbackType::Enum type,
    int id, nvn::DebugCallbackSeverity::Enum severity, const char* message,
    void* userParam) NN_NOEXCEPT
{
    if (s_levelMap.empty())
    {
        s_levelMap[nvn::DebugCallbackSeverity::NOTIFICATION] =
            spdlog::level::info;
        s_levelMap[nvn::DebugCallbackSeverity::LOW] = spdlog::level::warn;
        s_levelMap[nvn::DebugCallbackSeverity::MEDIUM] = spdlog::level::err;
        s_levelMap[nvn::DebugCallbackSeverity::HIGH] = spdlog::level::critical;
    }

    SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(), s_levelMap[severity],
                       "NVN {} message from {} ID {}: {}", type, source, id,
                       message);
}

void SwitchBackend::SetupImage(Image& image)
{
}

void SwitchBackend::CleanupImage(Image& image)
{
}

bool SwitchBackend::Update(Input::State& input)
{

    return true;
}

bool SwitchBackend::BeginRender()
{
    return true;
}

void SwitchBackend::DrawImage(const Image& image, uint32_t x, uint32_t y,
                              float scaleX, float scaleY, uint32_t srcX,
                              uint32_t srcY, uint32_t srcWidth,
                              uint32_t srcHeight, glm::u8vec3 color)
{
}

void SwitchBackend::EndRender()
{
    m_frameCount++;
}

const std::string& SwitchBackend::DescribeSystem() const
{
    static std::string description;
    if (description.empty())
    {
        nn::oe::FirmwareVersionForDebug version;
        nn::oe::GetFirmwareVersionForDebug(&version);
        description = fmt::format("Horizon OS {}", version.string);
    }
    return description;
}
