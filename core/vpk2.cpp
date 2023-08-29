#include "vpk2.h"

// Taken from here:
// https://github.com/ValveResourceFormat/ValvePak/blob/master/ValvePak/ValvePak/Utilities/Crc32.cs
static uint32_t s_valveCrc32Table[] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

CORE_API uint32_t Core::Vpk::ValveCrc32(const std::vector<uint8_t>& data)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint8_t c : data)
    {
        crc = (crc >> 8) ^ s_valveCrc32Table[c ^ (crc & 0xFF)];
    }

    return ~crc;
}

CORE_API Core::Vpk::Vpk2::Vpk2()
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

CORE_API Core::Vpk::Vpk2::Vpk2(const std::string& path, bool create)
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
        Quit("Failed to read VPK directory {}", dirPath);
    }

    m_header = *(Vpk2Header*)directory.data();
    if (m_header.signature != VPK2_SIGNATURE)
    {
        Quit("Invalid signature 0x{:08X}, expected 0x{:08X}",
             m_header.signature, VPK2_SIGNATURE);
    }
    if (m_header.version != VPK2_VERSION)
    {
        Quit("Version {} does not match expected version {}", m_header.version,
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

CORE_API std::vector<uint8_t> Core::Vpk::Vpk2::Read(const std::string& path)
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

CORE_API bool Core::Vpk::Vpk2::Exists(const std::string& path)
{
    return m_files.find(path) != m_files.end();
}

// TODO: This implementation probably works fine, but it could be improved to do
// less allocations somehow.
CORE_API void Core::Vpk::Vpk2::Write(const std::string& path)
{
    if (!path.length() && !m_realPath.length())
    {
        SPDLOG_WARN("Ignoring request to write VPK file with no name");
    }

    if (!m_realPath.length() || path.length())
    {
        m_realPath = path + VPK_EXTENSION;
    }

    SPDLOG_INFO("Writing VPK file to {}", m_realPath);

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

CORE_API void Core::Vpk::Vpk2::AddFile(const std::string& path,
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
        Quit("Failed to open VPK archive {}", archivePath);
    }

    Vpk2DirectoryEntry entry;
    entry.crc = ValveCrc32(data);
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

CORE_API uint32_t Core::Vpk::Vpk2::ComputeCrc32(const std::string& path)
{
    std::vector<uint8_t> data = Read(path);
    return ValveCrc32(data);
}
