#pragma once

#include "fs.h"

namespace Core
{
namespace Vpk
{

constexpr const char* VPK_EXTENSION = ".vpk";
constexpr uint32_t VPK_EXTENSION_LENGTH = 4;
constexpr uint32_t VPK2_SIGNATURE = 0x55AA1234;
constexpr uint32_t VPK2_VERSION = 2;
constexpr uint16_t VPK2_SPECIAL_INDEX = 0x7FFF;
constexpr uint16_t VPK2_ENTRY_TERMINATOR = 0xFFFF;
constexpr uint32_t VPK2_CHUNK_MAX_SIZE = 209715200; // 200M

#pragma pack(push, 1)
struct Vpk2Header
{
    uint32_t signature;
    uint32_t version;
    uint32_t treeSize;
    uint32_t fileDataSize;
    uint32_t externalMd5Size;
    uint32_t md5Size;
    uint32_t signatureSize;
};

struct Vpk2DirectoryEntry
{
    uint32_t crc;
    uint16_t preloadSize;
    uint16_t archiveIndex;
    uint32_t offset;
    uint32_t length;
    uint16_t terminator;
};

struct Vpk2ExternalMd5Entry
{
    uint32_t archiveIndex;
    uint32_t startingOffset;
    uint32_t count;
    uint8_t md5[16];
};

struct Vpk2Md5
{
    uint8_t treeMd5[16];
    uint8_t externalMd5Md5[16];
    uint8_t unknown[16];
};

struct Vpk2Signature
{
    uint32_t publicKeySize;
    uint8_t* publicKey;
    uint32_t signatureSize;
    uint8_t* signature;
};
#pragma pack(pop)

// Calculate a CRC32 using the table Valve formats use
CORE_API uint32_t ValveCrc32(const std::vector<uint8_t>& data);

class CORE_API Vpk2 : public Filesystem::FileSource
{
  public:
    using container = std::map<std::string, Vpk2DirectoryEntry>;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;
    using reverse_iterator = container::reverse_iterator;
    using const_reverse_iterator = container::const_reverse_iterator;

    // Create a VPK. The path will be set when it's written the first time, and
    // files cannot be added until then.
    Vpk2();

    // Load or create a VPK
    Vpk2(const std::string& path, bool create = false);

    const Vpk2Header& GetHeader()
    {
        return m_header;
    }

    const std::string& GetRealPath()
    {
        return m_realPath;
    }

    std::vector<uint8_t> Read(const std::string& path);
    bool Exists(const std::string& path);

    // Write the VPK directory
    void Write(const std::string& path = "");

    // Add a file into the VPK
    void AddFile(const std::string& path, const std::vector<uint8_t>& data);

    // Compute the CRC32 of an entry
    uint32_t ComputeCrc32(const std::string& path);

    size_t GetFileCount()
    {
        return m_files.size();
    }

    iterator begin()
    {
        return m_files.begin();
    }

    iterator end()
    {
        return m_files.end();
    }

    const_iterator begin() const
    {
        return m_files.begin();
    }

    const_iterator end() const
    {
        return m_files.end();
    }

  private:
    std::string m_realPath;

    Vpk2Header m_header;
    std::vector<Vpk2ExternalMd5Entry> m_externalMd5Section;
    Vpk2Md5 m_md5;
    Vpk2Signature m_signature;

    std::map<std::string, Vpk2DirectoryEntry> m_files;
    uint16_t m_currentArchive;
    uint32_t m_currentOffset;

    std::string GetBaseName()
    {
        return m_realPath.find(VPK_EXTENSION) > m_realPath.length()
                   ? m_realPath
                   : m_realPath.substr(0, m_realPath.length() -
                                              VPK_EXTENSION_LENGTH);
    }

    std::string GetDirectoryPath()
    {
        return fmt::format("{}_dir{}", GetBaseName(), VPK_EXTENSION);
    }

    std::string GetArchivePath(uint32_t archiveIndex = UINT32_MAX)
    {
        return fmt::format("{}_{:03}{}", GetBaseName(),
                           archiveIndex == UINT32_MAX ? m_currentArchive
                                                      : archiveIndex,
                           VPK_EXTENSION);
    }
};

} // namespace Vpk
} // namespace Core