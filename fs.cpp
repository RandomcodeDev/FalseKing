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

void PackEntry::Write(std::fstream& file) const
{
    uint64_t nameLength = name.length();
    file.write((char*)&nameLength, sizeof(uint64_t));
    file.write((char*)name.data(), name.length());
    file.write((char*)hash, sizeof(hash));
    file.write((char*)&rawSize, sizeof(uint64_t));
    file.write((char*)&realSize, sizeof(uint64_t));
    file.write((char*)&offset, sizeof(uint64_t));
}

PackTable::PackTable(fs::path path, bool create)
{
    SPDLOG_INFO("Loading pack file {}", path.string());

    tableFile.open(path.replace_extension(PACK_TABLE_EXTENSION),
                   std::ios::ate | std::ios::binary | std::ios::noreplace |
                       std::ios::in | std::ios::out);
    if (!tableFile.is_open())
    {
        Quit(fmt::format("Failed to open file {}", path.string()), errno);
    }
    path.replace_extension();
    dataFile.open(path.replace_extension(PACK_DATA_EXTENSION),
                  std::ios::ate | std::ios::binary | std::ios::noreplace |
                      std::ios::in | std::ios::out);
    if (!dataFile.is_open())
    {
        Quit(fmt::format("Failed to open file {}", path.string()), errno);
    }
    path.replace_extension();

    if (create)
    {
        if (tableFile.tellg() == 0)
        {
            SPDLOG_INFO("No table found, creating it");
            tableFile.write((char*)PACK_TABLE_MAGIC, PACK_TABLE_MAGIC_SIZE);

            // entry count
            uint64_t dummy = 0;
            tableFile.write((char*)&dummy, sizeof(uint64_t));
        }
        if (dataFile.tellg() == 0)
        {
            SPDLOG_INFO("No data file found, creating it");
            dataFile.write((char*)PACK_DATA_MAGIC, PACK_DATA_MAGIC_SIZE);
        }
    }
    else
    {
        if (tableFile.tellg() == 0)
        {
            fs::remove(path.replace_extension(PACK_TABLE_EXTENSION));
            Quit(fmt::format("Failed to read pack file {}", path.string()),
                 errno);
        }

        tableFile.read((char*)magic, PACK_TABLE_MAGIC_SIZE);
        if (memcmp(magic, PACK_TABLE_MAGIC, PACK_TABLE_MAGIC_SIZE) != 0)
        {
            Quit(fmt::format(
                "Failed to read pack file {}: invalid table magic {}",
                path.string(), (char*)magic));
        }

        uint64_t entryCount = 0;
        tableFile.read((char*)&entryCount, sizeof(uint64_t));
        for (uint64_t i = 0; i < entryCount; i++)
        {
            entries.push_back(PackEntry::Read(tableFile));
        }

        if (dataFile.tellg() == 0)
        {
            fs::remove(path.replace_extension(PACK_DATA_EXTENSION));
            Quit(fmt::format("Failed to read data file {}: {}", path.string(),
                             errno),
                 errno);
        }

        dataFile.read((char*)magic, PACK_DATA_MAGIC_SIZE);
        if (memcmp(magic, PACK_DATA_MAGIC, PACK_DATA_MAGIC_SIZE) != 0)
        {
            Quit(fmt::format(
                "Failed to read pack file {}: invalid data magic {}",
                path.string(), (char*)magic));
        }
    }

    SPDLOG_INFO("Pack file {} successfully loaded", path.string());
}

PackEntry* PackTable::FindEntry(const std::string& name)
{
    uint64_t hash = 0;

    MetroHash64::Hash((uint8_t*)name.data(), name.length(), (uint8_t*)&hash);
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

void PackTable::Update(const std::string& name,
                       const std::vector<uint8_t>& data, bool mustExist)
{
    PackEntry* entry = FindEntry(name);
    if (mustExist && !entry)
    {
        SPDLOG_WARN(
            "Couldn't find entry {}, but Update was called with mustExist");
        return;
    }

    if (!entry)
    {
        size_t offset = entries.size() > 0
                            ? entries.back().offset + entries.back().rawSize
                            : 0;
        entries.resize(entries.size() + 1);
        entry = &entries.back();
        entry->name = name;
        int64_t slash;
        while ((slash = entry->name.find('\\')) >= 0)
        {
            entry->name[slash] = '/';
        }
        entry->offset = offset;
    }
    MetroHash64::Hash((uint8_t*)name.c_str(), name.length(),
                      (uint8_t*)&entry->nameHash);
    MetroHash128::Hash(data.data(), data.size(), entry->hash);
    entry->realSize = data.size();

    std::vector<uint8_t> compressedData(ZSTD_COMPRESSBOUND(data.size()));
    size_t size = ZSTD_compress(compressedData.data(), compressedData.size(),
                                data.data(), data.size(), 9);
    if (ZSTD_isError(size))
    {
        SPDLOG_ERROR("Failed to compress data for pack entry {}: {}", name,
                     size);
        return;
    }

    // Shift over entries after the first one
    if (entries.size() > 1)
    {
        int64_t delta = size - entry->rawSize;
        entry->rawSize = size;
        dataFile.seekg(std::ios::end);
        size_t oldSize = (size_t)dataFile.tellg() - entry->offset;
        dataFile.seekg(entry->offset);
        size_t index = entry - entries.data();
        size_t endIndex = entries.size() - 1;
        std::vector<uint8_t> chunk;
        for (size_t i = endIndex; i >= index; i--)
        {
            size_t chunkSize =
                std::min(oldSize - entry->offset, MAX_SHIFT_CHUNK);
            chunk.reserve(chunkSize);
            dataFile.read((char*)chunk.data(), chunk.size());
            entries[i].offset += delta;
            dataFile.seekg(entries[i].offset);
            dataFile.write((char*)chunk.data(), chunk.size());
        }
    }

    dataFile.seekg(entry->offset);
    dataFile.write((char*)compressedData.data(), compressedData.size());
}

void PackTable::Append(const std::string& name,
                       const std::vector<uint8_t>& data)
{
    Update(name, data);
}

void PackTable::Save()
{
    uint64_t size = entries.size();
    tableFile.write((char*)&size, sizeof(uint64_t));
    for (const auto& entry : entries)
    {
        entry.Write(tableFile);
    }
}

std::vector<fs::path> Filesystem::s_paths;
std::vector<PackTable> Filesystem::s_tables;

void Filesystem::Initialize(const std::vector<fs::path>& paths)
{
    SPDLOG_INFO("Initializing filesystem");

    SPDLOG_INFO("Search paths: {}", paths.size());
    for (auto path : paths)
    {
        SPDLOG_INFO("\t{}{}", path.string(),
                    fs::exists(path) ? "" : " (not available)");
        if (path.extension() == PACK_BASE_EXTENSION)
        {
            if (fs::exists(path.replace_extension(PACK_TABLE_EXTENSION)))
            {
                s_tables.push_back(PackTable(path));
                SPDLOG_INFO("\t\tPack with {} files",
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
