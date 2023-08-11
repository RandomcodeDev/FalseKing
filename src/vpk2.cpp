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

    if (create)
    {
        m_realPath = path;

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

        SPDLOG_INFO("Creating VPK file {}", m_realPath);
        return;
    }
    else
    {
        SPDLOG_INFO("Loading VPK file {}", m_realPath);
    }

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
                currentOffset += sizeof(Vpk2DirectoryEntry) +
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

// TODO: This implementation probably works fine, but it could be improved to do
// less allocations somehow.
void Vpk::Vpk2::Write(const std::string& path, const std::string& extension)
{
    if (!path.length() && !m_realPath.length())
    {
        SPDLOG_WARN("Ignoring request to write VPK file with no name");
    }

    SPDLOG_INFO("Writing VPK file to {}{}", path, extension);

    if (!m_realPath.length() || path.length())
    {
        m_realPath = path + extension;
    }
    std::string dirPath = fmt::format(
        "{}_dir.vpk", m_realPath.substr(0, m_realPath.length() - 4));
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
        std::string path = lastSlash > filePath.length()
                               ? " "
                               : filePath.substr(0, lastSlash - 1);

        std::string name = filePath.substr(
            lastSlash > filePath.length() ? 0 : lastSlash, lastDot);

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
        directory.resize(directory.size() + extension.size() + 1);
        std::copy(extension.begin(), extension.begin() + extension.size(),
                  directory.begin() + oldSize);
        for (const auto& names : paths.second)
        {
            const std::string& path = names.first;
            oldSize = directory.size();
            directory.resize(directory.size() + path.size() + 1);
            std::copy(path.begin(), path.begin() + path.size(),
                      directory.begin() + oldSize);
            for (const auto& entry : names.second)
            {
                const std::string& name = entry.first;
                const Vpk2DirectoryEntry& directoryEntry = entry.second;
                oldSize = directory.size();
                directory.resize(directory.size() + name.size() + 1 +
                                 sizeof(Vpk2DirectoryEntry));
                std::copy(name.begin(), name.begin() + name.size(),
                          directory.begin() + oldSize);
                std::copy((uint8_t*)&directoryEntry,
                          (uint8_t*)(&directoryEntry + 1),
                          directory.begin() + oldSize + name.size());
            }

            // Add a NUL to terminate this level
            directory.resize(directory.size() + 1);
        }

        directory.resize(directory.size() + 1);
    }

    directory.resize(directory.size() + 1);

    m_header.treeSize =
        (uint32_t)(directory.size() - sizeof(Vpk2Header));
    std::copy((uint8_t*)&m_header, (uint8_t*)(&m_header + 1),
              directory.begin());
    directory.resize(directory.size() + sizeof(Vpk2Md5));
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
    SPDLOG_INFO("Adding {}-byte file as {}", data.size(), path);

    if (!m_realPath.length())
    {
        SPDLOG_WARN(
            "Ignoring attempt to add a file to a VPK file with no name");
        return;
    }

    std::string archivePath =
        // dir.vpk is 7 characters
        fmt::format("{}_{:03}.vpk",
                    m_realPath.substr(0, m_realPath.length() - 4),
                    m_currentArchive);
    SPDLOG_INFO("Checking if file will fit in {}", archivePath);

    if (m_currentOffset + data.size() > VPK2_CHUNK_MAX_SIZE)
    {
        m_currentArchive++;
        m_currentOffset = 0;

        archivePath = fmt::format("{}{:03}.vpk",
                                  m_realPath.substr(0, m_realPath.length() - 7),
                                  m_currentArchive);
        SPDLOG_INFO("File is {} byte(s) too large, writing to {} instead. "
                    "Previous archive padded {} byte(s).",
                    m_currentOffset + data.size() - VPK2_CHUNK_MAX_SIZE,
                    archivePath, VPK2_CHUNK_MAX_SIZE - m_currentOffset);
    }

    std::ofstream archive(archivePath, std::ios::binary);

    Vpk2DirectoryEntry entry;
    entry.crc = 0;
    entry.preloadSize = 0;
    entry.archiveIndex = m_currentArchive;
    entry.entryOffset = m_currentOffset;
    entry.entryLength = (uint32_t)data.size();

    archive.seekp(m_currentOffset, std::ios::beg);
    archive.write((const char*)data.data(), data.size());

    m_files[path] = entry;
}
