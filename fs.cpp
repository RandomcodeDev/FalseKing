#include "fs.h"

PackEntry PackEntry::Read(std::fstream& file)
{
    PackEntry entry{};

    uint64_t nameLength = 0;
    file.read((char*)&nameLength, sizeof(uint64_t));
    entry.name.resize(nameLength);
    file.read((char*)entry.name.data(), entry.name.size());
    file.read((char*)entry.hash, sizeof(entry.hash));
    file.read((char*)&entry.rawSize, sizeof(uint64_t));
    file.read((char*)&entry.realSize, sizeof(uint64_t));
    file.read((char*)&entry.offset, sizeof(uint64_t));

    return entry;
}

PackTable::PackTable(const std::string& path)
    : tableFile(path + PACK_TABLE_EXTENSION),
      dataFile(path + PACK_DATA_EXTENSION, std::ios::binary)
{
    SPDLOG_INFO("Loading pack file {}", path);

    // This can quit because it's only called if the pack exists, which is
    // enough reason to expect it to be valid
    if (!tableFile.good())
    {
        Quit(fmt::format("Failed to read pack file {}{}: {}", path,
                         PACK_TABLE_EXTENSION, errno),
             errno);
    }

    tableFile.read((char*)magic, PACK_TABLE_MAGIC_SIZE);
    if (memcmp(magic, PACK_TABLE_MAGIC, PACK_TABLE_MAGIC_SIZE) != 0)
    {
        Quit(fmt::format("Failed to read pack file {}: invalid table magic {}",
                         path, (char*)magic));
    }

    tableFile.read((char*)&dataSize, sizeof(uint64_t));

    uint64_t entryCount = 0;
    tableFile.read((char*)&entryCount, sizeof(uint64_t));
    for (uint64_t i = 0; i < entryCount; i++)
    {
        entries.push_back(PackEntry::Read(tableFile));
    }

    if (!dataFile.good())
    {
        Quit(fmt::format("Failed to read data file {}{}: {}", path,
                         PACK_DATA_EXTENSION, errno),
             errno);
    }

    dataFile.read((char*)magic, PACK_DATA_MAGIC_SIZE);
    if (memcmp(magic, PACK_DATA_MAGIC, PACK_DATA_MAGIC_SIZE) != 0)
    {
        Quit(fmt::format("Failed to read pack file {}: invalid data magic {}",
                         path, (char*)magic));
    }

    SPDLOG_INFO("Pack file {} successfully loaded", path);
}

PackEntry* PackTable::FindEntry(const std::string& name)
{
    uint64_t hash = 0;

    MetroHash64::Hash((uint8_t*)name.data(), name.length(), (uint8_t*)hash);
    for (auto& entry : entries)
    {
        if (entry.nameHash == hash)
        {
            return &entry;
        }
    }

    return nullptr;
}

std::vector<uint8_t> PackTable::Read(const std::string& name)
{
    PackEntry* entry = FindEntry(name);
    if (!entry)
    {
        SPDLOG_ERROR("Failed to locate entry {}", name);
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> rawData(entry->rawSize);
    dataFile.seekg(entry->offset);
    dataFile.read((char*)rawData.data(), rawData.size());

    std::vector<uint8_t> data(entry->realSize);
    ZSTD_decompress(data.data(), data.size(), rawData.data(), rawData.size());

    uint8_t hash[32];
    MetroHash128::Hash(data.data(), data.size(), hash);
    if (memcmp(entry->hash, hash, sizeof(hash)) != 0)
    {
        SPDLOG_ERROR("Hash of decompressed data for entry {} does not match", name);
        return std::vector<uint8_t>();
    }

    return data;
}

std::vector<std::string> Filesystem::s_paths;
std::vector<PackTable> Filesystem::s_tables;

void Filesystem::Initialize(const std::string& paths)
{
    size_t offset = 0;
    size_t last_offset = 0;
    std::vector<std::string> pathVector;
    while ((offset = paths.find(':', offset)) > 0)
    {
        pathVector.push_back(std::string(paths.substr(last_offset, offset)));
        last_offset = offset;
    }

    Initialize(pathVector);
}

void Filesystem::Initialize(const std::vector<std::string>& paths)
{
    SPDLOG_INFO("Initializing filesystem");

    s_paths = paths;

    SPDLOG_INFO("Search paths: {}", s_paths.size());
    for (const auto& path : s_paths)
    {
        SPDLOG_INFO("\t{}{}", path, fs::exists(path) ? "" : " (not available)");
        if (path.ends_with(PACK_BASE_EXTENSION))
        {
            if (fs::exists(path + PACK_TABLE_EXTENSION))
            {
                s_tables.push_back(PackTable(path));
                SPDLOG_INFO("\t\t{}-byte pack with {} files",
                            (*s_tables.end()).dataSize,
                            (*s_tables.end()).entries.size());
            }
        }
    }

    SPDLOG_INFO("Filesystem initialized");
}

std::vector<uint8_t> Filesystem::Read(const std::string& path)
{
    SPDLOG_TRACE("Reading file {}", path);

    for (auto& table : s_tables)
    {
        std::vector<uint8_t> data = table.Read(path);
        if (data.size() > 0)
        {
            return data;
        }
    }

    std::ifstream file(path);
    if (file.is_open())
    {
        file.seekg(std::ios::end);
        std::vector<uint8_t> data(file.tellg());
        file.seekg(std::ios::beg);
        file.read((char*)data.data(), data.size());
        return data;
    }

    return std::vector<uint8_t>();
}
