#include "vpk.h"

Vpk::Vpk2::Vpk2(const std::string& path)
{
    m_realPath = path;

    SPDLOG_INFO("Loading VPK file {}", m_realPath);

    m_realPath.insert(m_realPath.length() - 4, "_dir");
    std::vector<uint8_t> directory = Filesystem::Read(m_realPath);
    if (!directory.size())
    {
        QUIT("Failed to read VPK directory {}", m_realPath);
    }

    memcpy((void*)&m_header, (void*)directory.data(), sizeof(Vpk2Header));
    if (m_header.signature != VPK2_SIGNATURE)
    {
        QUIT("Invalid signature 0x{:08X}, expected 0x{:08X}",
             m_header.signature, VPK2_SIGNATURE);
    }
    if (m_header.version != VPK2_VERSION)
    {
        QUIT("Version {} does not match expected version {}", m_header.version,
             VPK2_VERSION);
    }
}

std::vector<uint8_t> Vpk::Vpk2::Read(const std::string& path)
{
    return std::vector<uint8_t>();
}

bool Vpk::Vpk2::Exists(const std::string& path)
{
    return m_files.find(path) != m_files.end();
}
