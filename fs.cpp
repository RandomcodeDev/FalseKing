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

PackTable::PackTable(fs::path path)
{
    SPDLOG_INFO("Loading pack file {}", path.string());

    tableFile.open(path.replace_extension(PACK_TABLE_EXTENSION));
    dataFile.open(path.replace_extension(PACK_DATA_EXTENSION));

    // This can quit because it's only called if the pack exists, which is
    // enough reason to expect it to be valid
    if (!tableFile.good())
    {
        Quit(fmt::format("Failed to read pack file {}{}: {}", path.string(),
                         PACK_TABLE_EXTENSION, errno),
             errno);
    }

    tableFile.read((char*)magic, PACK_TABLE_MAGIC_SIZE);
    if (memcmp(magic, PACK_TABLE_MAGIC, PACK_TABLE_MAGIC_SIZE) != 0)
    {
        Quit(fmt::format("Failed to read pack file {}: invalid table magic {}",
                         path.string(), (char*)magic));
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
        Quit(fmt::format("Failed to read data file {}{}: {}", path.string(),
                         PACK_DATA_EXTENSION, errno),
             errno);
    }

    dataFile.read((char*)magic, PACK_DATA_MAGIC_SIZE);
    if (memcmp(magic, PACK_DATA_MAGIC, PACK_DATA_MAGIC_SIZE) != 0)
    {
        Quit(fmt::format("Failed to read pack file {}: invalid data magic {}",
                         path.string(), (char*)magic));
    }

    SPDLOG_INFO("Pack file {} successfully loaded", path.string());
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
        SPDLOG_ERROR("Hash of decompressed data for entry {} does not match",
                     name);
        return std::vector<uint8_t>();
    }

    return data;
}

void Update(const std::string& name, const std::vector<uint8_t>& data)
{
}

void Append(const std::string& name, const std::vector<uint8_t>& data)
{
}

std::vector<fs::path> Filesystem::s_paths;
std::vector<PackTable> Filesystem::s_tables;

void Filesystem::Initialize(const std::vector<fs::path>& paths)
{
    SPDLOG_INFO("Initializing filesystem");

    SPDLOG_INFO("Search paths: {}", paths.size());
    for (auto path : paths)
    {
        SPDLOG_INFO("\t{}{}", path.string(), fs::exists(path) ? "" : " (not available)");
        if (path.extension() == PACK_BASE_EXTENSION)
        {
            if (fs::exists(path.replace_extension(PACK_TABLE_EXTENSION)))
            {
                s_tables.push_back(PackTable(path));
                SPDLOG_INFO("\t\t{}-byte pack with {} files",
                            (*s_tables.end()).dataSize,
                            (*s_tables.end()).entries.size());
            }
        }
        else
        {
            s_paths.push_back(path);
        }
    }

    SPDLOG_INFO("Filesystem initialized");
}

std::vector<uint8_t> Filesystem::Read(const fs::path& path)
{
    SPDLOG_TRACE("Reading file {}", path);

    for (auto& table : s_tables)
    {
        std::vector<uint8_t> data = table.Read(path.string());
        if (data.size() > 0)
        {
            return data;
        }
    }

    for (auto& root : s_paths)
    {
        std::ifstream file(root / path, std::ios::ate);
        if (file.is_open())
        {
            std::vector<uint8_t> data(file.tellg());
            file.seekg(std::ios::beg);
            file.read((char*)data.data(), data.size());
            return data;
        }
    }

    return std::vector<uint8_t>();
}
