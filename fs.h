#pragma once
#pragma once

#include "game.h"

// Abstracts the filesystem, works on an archive file or raw folders
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
};
