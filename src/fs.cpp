#include "fs.h"

static std::vector<fs::path> s_paths;

void Filesystem::Initialize(const std::vector<fs::path>& paths)
{
    SPDLOG_INFO("Initializing filesystem");

    SPDLOG_INFO("Search paths: {}", paths.size());
    for (auto path : paths)
    {
        SPDLOG_INFO("\t{}{}", path.string(),
                    fs::exists(path) ? "" : " (not available)");
        if (fs::exists(path))
        {
            s_paths.push_back(path);
        }
    }
    s_paths.push_back("");
    SPDLOG_INFO("{}/{} search paths available", s_paths.size(), paths.size());

    SPDLOG_INFO("Filesystem initialized");
}

std::vector<uint8_t> Filesystem::Read(const fs::path& path)
{
    SPDLOG_TRACE("Reading file {}", path);

    for (auto& root : s_paths)
    {
        std::ifstream file(root / path, std::ios::ate);
        if (file.is_open())
        {
            std::vector<uint8_t> data((size_t)file.tellg());
            file.seekg(std::ios::beg);
            file.read((char*)data.data(), data.size());
            return data;
        }
    }

    return std::vector<uint8_t>();
}