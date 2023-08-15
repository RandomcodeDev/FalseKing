#include "vpk2.h"

Vpk::Vpk2::Vpk2()
{
    m_header.signature = VPK2_SIGNATURE;
    m_header.version = VPK2_VERSION;
    m_header.treeSize = 0;
    m_header.fileDataSize = 0;
    m_header.externalMd5Size = 0;
    m_header.md5Size = sizeof(Vpk2Md5);
    m_header.signatureSize = 0;

    // TODO: implement MD5 stuff
    m_md5 = {};
    m_signature = {};

    m_currentArchive = 0;
    m_currentOffset = 0;
}

Vpk::Vpk2::Vpk2(const std::string& path, bool create)
{
    m_realPath = path;

    m_currentArchive = 0;
    m_currentOffset = 0;

    if (create)
    {
        m_header.signature = VPK2_SIGNATURE;
        m_header.version = VPK2_VERSION;
        m_header.treeSize = 0;
        m_header.fileDataSize = 0;
        m_header.externalMd5Size = 0;
        m_header.md5Size = sizeof(Vpk2Md5);
        m_header.signatureSize = 0;

        // TODO: implement MD5 stuff
        m_md5 = {};
        m_signature = {};

        SPDLOG_INFO("Creating VPK file {}", m_realPath);
        return;
    }
    else
    {
        SPDLOG_INFO("Loading VPK file {}", m_realPath);
    }

    std::string dirPath = GetDirectoryPath();
    std::vector<uint8_t> directory = Filesystem::Read(dirPath);
    if (directory.size() < sizeof(Vpk2Header))
    {
        QUIT("Failed to read VPK directory {}", dirPath);
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
                if (m_files[fullPath].offset > m_currentOffset)
                {
                    m_currentOffset = m_files[fullPath].offset;
                }
                if (m_files[fullPath].archiveIndex > m_currentArchive)
                {
                    m_currentArchive = m_files[fullPath].archiveIndex;
                    m_currentOffset = m_files[fullPath].offset;
                }
                currentOffset +=
                    sizeof(Vpk2DirectoryEntry) + m_files[fullPath].preloadSize;
                SPDLOG_DEBUG("Got entry {}", fullPath);
            }
        }
    }

    currentOffset += m_header.fileDataSize;

    size_t md5EntryCount =
        m_header.externalMd5Size / sizeof(Vpk2ExternalMd5Entry);
    m_externalMd5Section.resize(md5EntryCount, {});
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
        std::string archivePath = GetArchivePath();
        std::vector<uint8_t> archive = Filesystem::Read(archivePath);
        if (entry.offset + entry.length <= archive.size())
        {
            return std::vector<uint8_t>(archive.begin() + entry.offset,
                                        archive.begin() + entry.offset +
                                            entry.length);
        }
    }

    return std::vector<uint8_t>();
}

bool Vpk::Vpk2::Exists(const std::string& path)
{
    return m_files.find(path) != m_files.end();
}

// TODO: This implementation probably works fine, but it could be improved to do
// less allocations somehow.
void Vpk::Vpk2::Write(const std::string& path)
{
    if (!path.length() && !m_realPath.length())
    {
        SPDLOG_WARN("Ignoring request to write VPK file with no name");
    }

    SPDLOG_INFO("Writing VPK file to {}{}", path, VPK_EXTENSION);

    if (!m_realPath.length() || path.length())
    {
        m_realPath = path + VPK_EXTENSION;
    }
    std::string dirPath = GetDirectoryPath();
    std::ofstream file(dirPath, std::ios::binary);

    SPDLOG_INFO("Creating directory tree");

    std::unordered_map<
        std::string,
        std::unordered_map<std::string,
                           std::unordered_map<std::string, Vpk2DirectoryEntry>>>
        nameTree;

    for (const auto& file : m_files)
    {
        const std::string& filePath = file.first;
        const Vpk2DirectoryEntry& entry = file.second;

        size_t lastDot = filePath.rfind('.');
        std::string extension =
            lastDot > filePath.length()
                ? " "
                : filePath.substr(lastDot + 1, filePath.length() - lastDot - 1);

        size_t lastSlash = filePath.rfind('/', lastDot);
        std::string path =
            lastSlash > filePath.length() ? " " : filePath.substr(0, lastSlash);

        std::string name =
            filePath.substr(lastSlash < filePath.length() ? lastSlash + 1 : 0,
                            lastDot - lastSlash - 1);

        SPDLOG_DEBUG(
            "Adding file {} to the directory tree as \"{}\" \"{}\" \"{}\"",
            filePath, extension, path, name);
        nameTree[extension][path][name] = entry;
    }

    std::vector<uint8_t> directory(sizeof(Vpk2Header));
    for (const auto& paths : nameTree)
    {
        const std::string& extension = paths.first;
        size_t oldSize = directory.size();
        directory.resize(directory.size() + extension.size() + 1, {});
        std::copy(extension.begin(), extension.begin() + extension.size(),
                  directory.begin() + oldSize);
        for (const auto& names : paths.second)
        {
            const std::string& path = names.first;
            oldSize = directory.size();
            directory.resize(directory.size() + path.size() + 1, {});
            std::copy(path.begin(), path.begin() + path.size(),
                      directory.begin() + oldSize);
            for (const auto& entry : names.second)
            {
                const std::string& name = entry.first;
                const Vpk2DirectoryEntry& directoryEntry = entry.second;
                oldSize = directory.size();
                directory.resize(directory.size() + name.size() + 1 +
                                     sizeof(Vpk2DirectoryEntry),
                                 {});
                std::copy(name.begin(), name.begin() + name.size(),
                          directory.begin() + oldSize);
                // TODO: figure out where these 4 bytes come from and annihilate
                // them
                std::copy((uint8_t*)&directoryEntry,
                          (uint8_t*)(&directoryEntry + 1),
                          directory.begin() + oldSize + name.size() + 1);
            }

            // Add a NUL to terminate this level
            directory.resize(directory.size() + 1, {});
        }

        directory.resize(directory.size() + 1, {});
    }

    directory.resize(directory.size() + 1, {});

    m_header.treeSize = (uint32_t)(directory.size() - sizeof(Vpk2Header));
    std::copy((uint8_t*)&m_header, (uint8_t*)(&m_header + 1),
              directory.begin());
    directory.resize(directory.size() + sizeof(Vpk2Md5), {});
    std::copy((uint8_t*)&m_md5, (uint8_t*)(&m_md5 + 1),
              directory.end() - sizeof(Vpk2Md5));

    SPDLOG_INFO("Writing directory. Directory tree is {} byte(s), directory is "
                "{} byte(s).",
                m_header.treeSize, directory.size());

    file.write((const char*)directory.data(), directory.size());
}

void Vpk::Vpk2::AddFile(const std::string& path,
                        const std::vector<uint8_t>& data)
{
    std::string cleanPath = Filesystem::CleanPath(path);
    SPDLOG_INFO("Adding {}-byte file as {}", data.size(), cleanPath);

    if (!m_realPath.length())
    {
        SPDLOG_WARN(
            "Ignoring attempt to add a file to a VPK file with no name");
        return;
    }

    // Make sure not to exceed the chunk size unless the file is bigger than it
    if (m_currentOffset + data.size() > VPK2_CHUNK_MAX_SIZE)
    {
        m_currentArchive++;
        m_currentOffset = 0;
    }

    std::string archivePath = GetArchivePath();
    std::ofstream archive(archivePath, std::ios::binary | std::ios::app);
    if (!archive.is_open())
    {
        QUIT("Failed to open VPK archive {}", archivePath);
    }

    Vpk2DirectoryEntry entry;
    entry.crc = 0;
    entry.preloadSize = 0;
    entry.archiveIndex = m_currentArchive;
    entry.offset = m_currentOffset;
    entry.length = (uint32_t)data.size();
    entry.terminator = VPK2_ENTRY_TERMINATOR;

    archive.write((const char*)data.data(), data.size());
    archive.flush();

    SPDLOG_INFO("File added to {}, {} byte(s) written at offset 0x{:X}",
                archivePath, (size_t)archive.tellp() - m_currentOffset,
                m_currentOffset);
    m_currentOffset = (uint32_t)archive.tellp();

    m_files[cleanPath] = entry;
}
