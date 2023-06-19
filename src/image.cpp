#define QOI_IMPLEMENTATION

#include "image.h"
#include "backend.h"
#include "fs.h"

Image::Image(Backend* backend, const std::string& path) : m_backend(backend)
{
    std::vector<uint8_t> data = Filesystem::Read(path);
    m_data = qoi_decode(data.data(), (int32_t)data.size(), &m_info, 4);
    if (!m_data)
    {
        Quit(fmt::format("Failed to read image {}", path));
    }

    m_backend->SetupImage(*this);
}

Image::~Image()
{
    m_backend->CleanupImage(*this);
    free(m_data);
}
