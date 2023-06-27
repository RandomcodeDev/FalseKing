#pragma once
#pragma once

#include "game.h"

// Abstracts the filesystem, works on an archive file or raw folders
namespace Filesystem
{
// Initialize the filesystem
extern void Initialize(const std::vector<std::string>& paths);

// Read a file
extern std::vector<uint8_t> Read(const std::string& path);

// Write a file
extern void Write(const std::string& path, const std::vector<uint8_t>& data);

// Check if a file can be opened
extern bool Exists(const std::string& path);
}; // namespace Filesystem
