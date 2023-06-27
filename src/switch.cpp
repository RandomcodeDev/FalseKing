#include <nn/fs.h>
#include <nn/nn_Log.h>
#include <nn/oe.h>
#include <nn/vi.h>

#include "backend.h"
#include "image.h"
#include "input.h"
#include "sprite.h"

/*
template <typename Mutex>
class SwitchSink : public spdlog::sinks::base_sink<Mutex>
{
  public:
    SwitchSink() {}

  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        formatted.push_back(
            '\0'); // add a null terminator for NN_LOG
        NN_PUT(formatted.begin(), formatted.size());
    }

    void flush_() override
    {
    }
};
*/

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
    KeyMapping& GetKeyMapping()
    {
        return m_mapping;
    }

    static const inline char* ROM_MOUNT = "rom";

  private:
    WindowInfo m_windowInfo;
    KeyMapping m_mapping;
    void* m_fsRomCache;
    size_t m_fsRomCacheSize;

    static const inline KeyMapping DUMMY_MAPPING = {};
};

extern "C" int nnMain()
{
    // stdout is already logged
    //spdlog::default_logger().get()->sinks().push_back(
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

SwitchBackend::SwitchBackend()
{
    m_mapping = DUMMY_MAPPING;

    nn::oe::Initialize();

    nn::fs::QueryMountRomCacheSize(&m_fsRomCacheSize);
    SPDLOG_INFO("Mounting RomFS with {}-byte cache to {}", m_fsRomCacheSize, ROM_MOUNT);
    m_fsRomCache = new uint8_t[m_fsRomCacheSize];
    if (!m_fsRomCache)
    {
        Quit("Failed to allocate RomFS cache");
    }
    nn::fs::MountRom(ROM_MOUNT, m_fsRomCache, m_fsRomCacheSize);
}

SwitchBackend::~SwitchBackend()
{
    nn::fs::Unmount(ROM_MOUNT);
    delete[] (uint8_t*)m_fsRomCache;
}

void SwitchBackend::SetupImage(Image& image)
{
}

void SwitchBackend::CleanupImage(Image& image)
{
}

bool SwitchBackend::Update(InputState& input)
{
    return false;
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
