#pragma once
#pragma once

#include "game.h"

// Abstracts the filesystem, works on an archive file or raw folders
namespace Filesystem
{
// Initialize the filesystem
extern void Initialize(const std::vector<fs::path>& paths);

// Read a file
extern std::vector<uint8_t> Read(const fs::path& path);

// Write a file
extern void Write(const fs::path& path, const std::vector<uint8_t>& data);
}; // namespace Filesystem
