#include "vpk.h"

Vpk::Vpk2::Vpk2(const std::string& path)
{
    m_realPath = path;

    SPDLOG_INFO("Loading VPK file {}", m_realPath);

    m_realPath.insert(m_realPath.length() - 4, "_dir");
    std::vector<uint8_t> directory = Filesystem::Read(m_realPath);
    if (directory.size() < sizeof(Vpk2Header))
    {
        QUIT("Failed to read VPK directory {}", m_realPath);
    }

    m_header = *(Vpk2Header*)directory.data();
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

    std::string extension;
    std::string insidePath;
    std::string name;
    size_t currentOffset = sizeof(Vpk2Header);

    auto readString = [&currentOffset, &directory]() -> std::string {
        std::string str;

        while (directory[currentOffset])
        {
            str += directory[currentOffset];
            currentOffset++;
        }
        currentOffset++;

        return str;
    };

    // Direct ripoff of the Valve Developer Wiki pseudocode, because this file
    // structure is goofy

    while (true)
    {
        extension = readString();
        if (!extension.length())
        {
            break;
        }

        while (true)
        {
            insidePath = readString();
            if (!insidePath.length())
            {
                break;
            }

            while (true)
            {
                name = readString();
                if (!name.length())
                {
                    break;
                }

                std::string fullPath =
                    (insidePath == " " ? "" : insidePath + "/") +
                    (name == " " ? "" : name) +
                    (extension == " " ? "" : "." + extension);
                m_files[fullPath] =
                    *(Vpk2DirectoryEntry*)(directory.data() + currentOffset);
                currentOffset += sizeof(Vpk2DirectoryEntry) - 2;
                SPDLOG_INFO("Got entry {}", fullPath);
            }
        }
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
