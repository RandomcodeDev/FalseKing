#include "backend.h"
#include "image.h"

Image::Image(Backend* backend, const std::string& path)
    : m_backend(backend)
{
    m_data = qoi_read(path.c_str(), &m_info, 4);
    if (!m_data)
    {
        Quit(fmt::format("Failed to read image {}", path), 1);
    }

    m_backend->SetupImage(*this);
}

Image::~Image()
{
    m_backend->CleanupImage(*this);
    free(m_data);
}
