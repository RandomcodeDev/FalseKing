#include <nn/fs.h>
#include <nn/gfx.h>
#include <nn/hid.h>
#include <nn/nn_Log.h>
#include <nn/oe.h>
#include <nn/oe/oe_DebugApis.h>
#include <nn/vi.h>

#include <nv/nv_MemoryManagement.h>

#include "backend.h"
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
    // unused, no keyboard support on switch
    KeyMapping& GetKeyMapping()
    {
        return m_mapping;
    }
    const std::string& DescribeSystem() const;

    static const inline char* ROM_MOUNT = "rom";

  private:
    WindowInfo m_windowInfo;
    KeyMapping m_mapping; // unused
    void* m_fsRomCache;
    size_t m_fsRomCacheSize;
    nn::vi::Display* m_display;
    nn::vi::Layer* m_layer;
    nn::gfx::Device m_device;

    // Graphics memory stuff

    // Initialize graphics objects
    void InitializeGraphics();
    void CreateDevice();

    // Destroy graphics objects
    void ShutdownGraphics();

    // Allocation functions
    static void* GraphicsAllocate(size_t size, size_t alignment, void* userData);
    static void GraphicsFree(void* memory, void* userData);
    static void* GraphicsReallocate(void* oldMemory, size_t newSize, void* userData);
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

void SwitchBackend::InitializeGraphics()
{
    SPDLOG_INFO("Initializing graphics");

    nn::gfx::InitializeNvn8();
    nv::SetGraphicsAllocator(GraphicsAllocate, GraphicsFree, GraphicsReallocate,
                             nullptr);
    nv::SetGraphicsDevtoolsAllocator(GraphicsAllocate, GraphicsFree, GraphicsReallocate,
                             nullptr);
    nv::InitializeGraphics(s_graphicsFirmwareMemory,
                           sizeof(s_graphicsFirmwareMemory));

    CreateDevice();

    SPDLOG_INFO("Graphics initialized");
}

void SwitchBackend::ShutdownGraphics()
{
    SPDLOG_INFO("Shutting down graphics");

    m_device.Finalize();

    nn::gfx::FinalizeNvn8();

    SPDLOG_INFO("Graphics shut down");
}

void SwitchBackend::CreateDevice()
{
    SPDLOG_INFO("Creating device");

    nn::gfx::DeviceInfo info;
    info.SetDefault();
    info.SetApiVersion(nn::gfx::ApiMajorVersion, nn::gfx::ApiMinorVersion);
#ifdef _DEBUG
    SPDLOG_INFO("Enabling debugging features for device");
    info.SetDebugMode(nn::gfx::DebugMode_Full);
#endif
    m_device.Initialize(info);
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

void* SwitchBackend::GraphicsReallocate(void* oldMemory, size_t newSize, void* userData)
{
    SPDLOG_TRACE("Reallocate graphics allocation 0x{} to {} bytes", oldMemory,
                 newSize);
    return realloc(oldMemory, newSize);
}

void SwitchBackend::SetupImage(Image& image)
{
}

void SwitchBackend::CleanupImage(Image& image)
{
}

bool SwitchBackend::Update(InputState& input)
{

    return true;
}

bool SwitchBackend::BeginRender()
{
    return false;
}

void SwitchBackend::DrawImage(const Image& image, uint32_t x, uint32_t y,
                              float scaleX, float scaleY, uint32_t srcX,
                              uint32_t srcY, uint32_t srcWidth,
                              uint32_t srcHeight, glm::u8vec3 color)
{
}

void SwitchBackend::EndRender()
{
}

const std::string& SwitchBackend::DescribeSystem() const
{
    nn::oe::FirmwareVersionForDebug version;
    return fmt::format("Horizon OS {}", version.string);
}
