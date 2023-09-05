#pragma once

#include "stdafx.h"

namespace Core
{
// Abstracts the filesystem
namespace Filesystem
{

// Represents a thing that files can be read from
class CORE_API FileSource
{
  public:
    static FileSource* Create(const std::string& path);

    virtual const std::string& GetRealPath() = 0;

    virtual std::vector<uint8_t> Read(const std::string& path) = 0;
    virtual bool Exists(const std::string& path) = 0;
};

// Clean a path
extern CORE_API std::string CleanPath(const std::string& path);

// Initialize the filesystem
extern CORE_API void Initialize(
    const std::vector<std::string>& searchPaths = std::vector<std::string>());

// Add a search path
extern CORE_API void AddSearchPath(const std::string& path);

// Read a file
extern CORE_API std::vector<uint8_t> Read(const std::string& path);

// Write a file
extern CORE_API void Write(const std::string& path,
                           const std::vector<uint8_t>& data);

// Check if a file can be opened
extern CORE_API bool Exists(const std::string& path);

// Resolve a path
extern CORE_API std::string ResolvePath(const std::string& path);
} // namespace Filesystem
} // namespace Core
