#pragma once

#include "game.h"

// Forward declarations
class Backend;

// Image
class Image
{
public:
    // Opaque backend information
    void* backendData;

    // Load the image at the specified path
    Image(Backend* backend, const std::string& path);

    // Free image memory
    ~Image();

    // Get the pixels in the image
    void* GetPixels() const
    {
        return m_data;
    }

    // Get the image dimensions
    void GetSize(uint32_t& width, uint32_t& height) const
    {
        width = m_info.width;
        height = m_info.height;
    }
private:
    Backend* m_backend;
    void* m_data;
    qoi_desc m_info;
};
