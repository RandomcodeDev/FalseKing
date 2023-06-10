#pragma once

#include "game.h"

// Pack file extensions
constexpr const char* PACK_BASE_EXTENSION = ".pak";
constexpr const char* PACK_TABLE_EXTENSION = "_table";
constexpr const char* PACK_DATA_EXTENSION = "_data";

// Pack table magic signature
constexpr uint8_t PACK_TABLE_MAGIC[] = "PACK0\0";
constexpr size_t PACK_TABLE_MAGIC_SIZE = sizeof(PACK_TABLE_MAGIC);

// Pack data magic signature
constexpr uint8_t PACK_DATA_MAGIC[] = "DATA0\0";
constexpr size_t PACK_DATA_MAGIC_SIZE = sizeof(PACK_DATA_MAGIC);

// Pack table entry
struct PackEntry
{
    std::string name;
    uint64_t nameHash;
    uint8_t hash[32];
    uint64_t rawSize;
    uint64_t realSize;
    uint64_t offset;

    // Read an entry
    static PackEntry Read(std::fstream& file);
};

// Pack file table, separate file for faster searching
struct PackTable
{
    uint8_t magic[PACK_TABLE_MAGIC_SIZE];
    uint64_t dataSize;
    uint8_t dataHash[32];
    std::vector<PackEntry> entries;

    std::fstream tableFile;
    std::fstream dataFile;

    // Load a pack table from the given path
    PackTable(const std::string& path);

    // Find an entry by name
    PackEntry* FindEntry(const std::string& name);

    // Read an entry's data
    std::vector<uint8_t> Read(const std::string& name);

    // Update an entry
    void Update(const std::string& name, const std::vector<uint8_t>& data);

    // Append an entry
    void Append(const std::string& name, const std::vector<uint8_t>& data);
};

// Abstracts the filesystem, works on a pack file or raw folders
class Filesystem
{
  public:
    // Initialize the filesystem
    static void Initialize(const std::string& paths);

    // Initialize the filesystem
    static void Initialize(const std::vector<std::string>& paths);

    // Read a file
    static std::vector<uint8_t> Read(const std::string& path);

    // Write a file
    static void Write(const std::string& path,
                      const std::vector<uint8_t>& data);

  private:
    static std::vector<std::string> s_paths;
    static std::vector<PackTable> s_tables;
};
