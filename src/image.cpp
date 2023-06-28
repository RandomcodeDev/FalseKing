#define QOI_IMPLEMENTATION

#include "image.h"
#include "backend.h"
#include "fs.h"

Image::Image(const std::string& path)
{
    std::vector<uint8_t> data = Filesystem::Read(path);
    m_data = qoi_decode(data.data(), (int32_t)data.size(), &m_info, 4);
    if (!m_data)
    {
        QUIT("Failed to read image {}", path);
    }

    g_backend->SetupImage(*this);
}

Image::~Image()
{
    g_backend->CleanupImage(*this);
    free(m_data);
}
