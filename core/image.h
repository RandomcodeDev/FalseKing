#pragma once

#include "stdafx.h"

namespace Core
{

// Forward declarations
class CORE_API Backend;

// Image
class CORE_API Image
{
  public:
    // This indicates the image was created in memory
    static constexpr const char* MEMORY_PATH = "<memory>";

    // Opaque backend information
    void* backendData;

    // Load the image at the specified path
    Image(const std::string& path);

    // Create an image from pixels
    Image(const uint8_t* pixels = DEFAULT_PIXELS, uint32_t width = 16,
          uint32_t height = 16, uint8_t channels = 4);

    Image(const Image& other)
    {
        *this = other;
    }

    // Free image memory
    ~Image();

    // Get the pixels in the image
    void* GetPixels() const
    {
        return m_data;
    }

    // Get the number of channels in the image
    uint8_t GetChannels()
    {
        return m_info.channels;
    }

    // Get whether the image is sRGB
    bool IsSrgb()
    {
        return m_info.colorspace == QOI_SRGB;
    }

    // Get the image dimensions
    void GetSize(uint32_t& width, uint32_t& height) const
    {
        width = m_info.width;
        height = m_info.height;
    }

    // Get the source of this image
    const std::string& GetSource() const
    {
        return m_source;
    }

  private:
    std::string m_source;
    void* m_data;
    qoi_desc m_info;

    static const uint8_t DEFAULT_PIXELS[16 * 16 * 4 + 1];
};

} // namespace Core