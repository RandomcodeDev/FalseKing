#include "vpk.h"

Vpk::Vpk2::Vpk2()
{
    m_header.signature = VPK2_SIGNATURE;
    m_header.version = VPK2_VERSION;
    m_header.treeSize = 0;
    m_header.fileDataSize = 0;
    m_header.externalMd5Size = 0;
    m_header.md5Size = sizeof(Vpk2Md5);
    m_header.signatureSize = 0;
}

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

    auto readString = [&]() -> std::string {
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
                currentOffset += sizeof(Vpk2DirectoryEntry) - 2 +
                                 m_files[fullPath].preloadSize;
                SPDLOG_DEBUG("Got entry {}", fullPath);
            }
        }
    }

    currentOffset += m_header.fileDataSize;

    size_t md5EntryCount =
        m_header.externalMd5Size / sizeof(Vpk2ExternalMd5Entry);
    m_externalMd5Section.resize(md5EntryCount);
    std::copy((Vpk2ExternalMd5Entry*)(directory.data() + currentOffset),
              (Vpk2ExternalMd5Entry*)(directory.data() + currentOffset) +
                  md5EntryCount,
              m_externalMd5Section.data());
    currentOffset += m_header.externalMd5Size;
    if (m_header.md5Size > 0)
    {
        m_md5 = *(Vpk2Md5*)(directory.data() + currentOffset);
        currentOffset += m_header.md5Size;
    }
    if (m_header.signatureSize > 0)
    {
        m_signature.publicKeySize =
            *(uint32_t*)(directory.data() + currentOffset);
        m_signature.publicKey = directory.data() + currentOffset;
        currentOffset += m_signature.publicKeySize;
        m_signature.signatureSize =
            *(uint32_t*)(directory.data() + currentOffset);
        m_signature.signature = directory.data() + currentOffset;
        currentOffset += m_signature.signatureSize;
    }
}

std::vector<uint8_t> Vpk::Vpk2::Read(const std::string& path)
{
    // TODO: maybe it's possible for a file to be in multiple chunks.
    // However, it's unlikely this game will have individual files more than
    // a few megabytes.
    if (m_files.find(path) != m_files.end())
    {
        Vpk2DirectoryEntry entry = m_files[path];
        std::string archivePath =
            // dir.vpk is 7 characters
            fmt::format("{}{:03}.vpk",
                        m_realPath.substr(0, m_realPath.length() - 7),
                        entry.archiveIndex);
        std::vector<uint8_t> archive = Filesystem::Read(archivePath);
        return std::vector<uint8_t>(archive.begin() + entry.entryOffset,
                                    archive.begin() + entry.entryOffset +
                                        entry.entryLength);
    }

    return std::vector<uint8_t>();
}

bool Vpk::Vpk2::Exists(const std::string& path)
{
    return m_files.find(path) != m_files.end();
}

void Vpk::Vpk2::Write(const std::string& path, const std::string& extension)
{
    std::string dirPath = fmt::format("{}_dir{}", path, extension);
    std::ofstream file(dirPath, std::ios::binary);

    auto makeTreeEntry = [this](const std::string& file)
    {

    };
}
