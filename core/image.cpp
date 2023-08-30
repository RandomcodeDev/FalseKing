#define QOI_IMPLEMENTATION

#include "image.h"
#include "backend.h"
#include "fs.h"

CORE_API Core::Image::Image(const std::string& path) : Image()
{
    std::vector<uint8_t> data = Filesystem::Read(path);
    if (data.size())
    {
        void* decodedData =
            qoi_decode(data.data(), (int32_t)data.size(), &m_info, 4);
        backendData = nullptr;
        if (!decodedData)
        {
            SPDLOG_ERROR("Failed to decode image {}", path);
        }
        else
        {
            free(m_data);
            m_data = decodedData;
            g_backend->SetupImage(*this);
        }
    }
    else
    {
        SPDLOG_ERROR("Failed to read image {}", path);
    }
}

CORE_API Core::Image::Image(const uint8_t* pixels, uint32_t width,
                            uint32_t height,
             uint8_t channels)
{
    m_data = calloc(width * height, channels);
    if (!m_data)
    {
        Quit("Failed to allocate data for image: {}",
                  strerror(errno), errno);
    }
    std::copy(pixels, pixels + width * height * channels, (uint8_t*)m_data);

    m_info = {width, height, channels, QOI_SRGB};

    backendData = nullptr;
    g_backend->SetupImage(*this);
}

CORE_API Core::Image::~Image()
{
    g_backend->CleanupImage(*this);
    free(m_data);
}

// magenta + black checker like Source/Minecraft/etc, exported from GIMP
const uint8_t Core::Image::DEFAULT_PIXELS[16 * 16 * 4 + 1] =
    ("\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000"
     "\377\377\377"
     "\000\377\377\377\000\377\377\377\000\377\377\000\000\000\377\000\000\000"
     "\377\000\000\000\377\000\000\000"
     "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\377"
     "\000\377\377\377\000\377\377\377"
     "\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377"
     "\377\377\000\377"
     "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377\000\000\000\377"
     "\000\000\000\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000"
     "\377\377\377\000\377"
     "\377\377\000\377\377\377\000\377\377\377\000\377\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\377\000\377\377\377\000\377"
     "\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377"
     "\000\377\377"
     "\377\000\377\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\000\000\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\377\000\377\377\377\000\377\377\377\000"
     "\377\377\377\000\377\377"
     "\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\000\000"
     "\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\000\000\000\377\377\000\377\377"
     "\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000"
     "\377\377\377"
     "\000\377\377\377\000\377\377\000\000\000\377\000\000\000\377\000\000\000"
     "\377\000\000\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\377\000\377\377\377\000"
     "\377\377\377\000\377\377\377"
     "\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377"
     "\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\000\000\000\377\000\000\000\377\377"
     "\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377"
     "\377\377\000\377"
     "\377\377\000\377\377\377\000\377\377\000\000\000\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000"
     "\377\000\000\000\377\000\000\000\377\000\000\000"
     "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\377"
     "\000\377\377\377\000\377\377\377"
     "\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377"
     "\377\377\000\377"
     "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377\000\000\000\377"
     "\000\000\000\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000"
     "\377\377\377\000\377"
     "\377\377\000\377\377\377\000\377\377\377\000\377\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\377\000\377\377\377\000\377"
     "\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377"
     "\000\377\377"
     "\377\000\377\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\000\000\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\377\000\377\377\377\000\377\377\377\000"
     "\377\377\377\000\377\377"
     "\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\000\000"
     "\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\000\000\000\377\377\000\377\377"
     "\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000"
     "\377\377\377"
     "\000\377\377\377\000\377\377\000\000\000\377\000\000\000\377\000\000\000"
     "\377\000\000\000\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\377\000\377\377\377\000"
     "\377\377\377\000\377\377\377"
     "\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377"
     "\377\000\000\000\377"
     "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
     "\000\377\000\000\000\377\000\000\000\377\377"
     "\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377"
     "\377\377\000\377"
     "\377\377\000\377\377\377\000\377\377\000\000\000\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377\000"
     "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\377\000\377"
     "\377\377\000\377\377\377\000\377"
     "\377\377\000\377\377\377\000\377\377\377\000\377\377\377\000\377\377\377"
     "\000\377\377");