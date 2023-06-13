#pragma once

#include "game.h"

// Pack file extensions
constexpr const char* PACK_BASE_EXTENSION = ".pak";
constexpr const char* PACK_TABLE_EXTENSION = ".pak_table";
constexpr const char* PACK_DATA_EXTENSION = ".pak_data";

// Pack table magic signature
constexpr uint8_t PACK_TABLE_MAGIC[] = "PACK0\0";
constexpr size_t PACK_TABLE_MAGIC_SIZE = sizeof(PACK_TABLE_MAGIC);

// Pack data magic signature
constexpr uint8_t PACK_DATA_MAGIC[] = "DATA0\0";
constexpr size_t PACK_DATA_MAGIC_SIZE = sizeof(PACK_DATA_MAGIC);

// Pack table entry
class PackEntry
{
  protected:
    friend class PackTable;
    friend class Filesystem;

    std::string name;
    uint64_t nameHash;
    uint8_t hash[32];
    uint64_t rawSize;
    uint64_t realSize;
    uint64_t offset;

    // Read an entry
    static PackEntry Read(std::fstream& file);

    // Write an entry
    void Write(std::fstream& file) const;
};

// Pack file table, separate file for faster searching
class PackTable
{
  protected:
    friend class Filesystem;

    uint8_t magic[PACK_TABLE_MAGIC_SIZE];
    std::vector<PackEntry> entries;

    std::fstream tableFile;
    std::fstream dataFile;

  public:
    // Load a pack table from the given path
    PackTable(fs::path path, bool create = false);

    // Find an entry by name
    PackEntry* FindEntry(const std::string& name);

    // Read an entry's data
    std::vector<uint8_t> Read(const std::string& name);

    // Update an entry
    void Update(const std::string& name, const std::vector<uint8_t>& data, bool mustExist = false);

    // Append an entry
    void Append(const std::string& name, const std::vector<uint8_t>& data);

    // Save the table
    void Save();

private:
    // Maximum amount to shift at once when updating data files
    static constexpr size_t MAX_SHIFT_CHUNK = 1024 * 1024 * 1024;
};

// Abstracts the filesystem, works on a pack file or raw folders
class Filesystem
{
  public:
    // Initialize the filesystem
    static void Initialize(const std::vector<fs::path>& paths);

    // Read a file
    static std::vector<uint8_t> Read(const fs::path& path);

    // Write a file
    static void Write(const fs::path& path, const std::vector<uint8_t>& data);

  private:
    static std::vector<fs::path> s_paths;
    static std::vector<PackTable> s_tables;
};
