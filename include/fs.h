#pragma once

#include "game.h"

// Abstracts the filesystem
namespace Filesystem
{

// Represents a thing that files can be read from
class FileSource
{
  public:
    static FileSource* Create(const std::string& path);

    virtual std::string GetRealPath() = 0;

    virtual std::vector<uint8_t> Read(const std::string& path) = 0;
    virtual bool Exists(const std::string& path) = 0;
};

// Clean a path
extern std::string CleanPath(const std::string& path);

// Initialize the filesystem
extern void Initialize(
    const std::vector<std::string>& searchPaths = std::vector<std::string>());

// Add a search path
extern void AddSearchPath(const std::string& path);

// Read a file
extern std::vector<uint8_t> Read(const std::string& path);

// Write a file
extern void Write(const std::string& path, const std::vector<uint8_t>& data);

// Check if a file can be opened
extern bool Exists(const std::string& path);
}; // namespace Filesystem
