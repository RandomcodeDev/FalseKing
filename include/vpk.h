#pragma once

#include "fs.h"

namespace Vpk
{

constexpr uint16_t VPK_ENTRY_TERMINATOR = 0xFFFF;

constexpr uint32_t VPK2_SIGNATURE = 0x55AA1234;
constexpr uint32_t VPK2_VERSION = 2;
constexpr uint16_t VPK2_SPECIAL_INDEX = 0x7FFF;

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
    uint32_t entryOffset;
    uint32_t entryLength;
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

class Vpk2 : public Filesystem::FileSource
{
  public:
    Vpk2();
    Vpk2(const std::string& path);
    ~Vpk2();

    std::string GetRealPath()
    {
        return m_realPath;
    }

    std::vector<uint8_t> Read(const std::string& path);
    bool Exists(const std::string& path);

    // Write the VPK
    void Write(const std::string& path, const std::string& extension = ".vpk");

    // Add a file into the VPK
    void AddFile(const std::string& path, const std::vector<uint8_t>& data);

  private:
    std::string m_realPath;

    Vpk2Header m_header;
    std::vector<Vpk2ExternalMd5Entry> m_externalMd5Section;
    Vpk2Md5 m_md5;
    Vpk2Signature m_signature;

    std::map<std::string, Vpk2DirectoryEntry> m_files;
};

} // namespace Vpk
