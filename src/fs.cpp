#include "fs.h"

static std::vector<std::string> s_paths;

void Filesystem::Initialize(const std::vector<std::string>& paths)
{
    SPDLOG_INFO("Initializing filesystem");

    SPDLOG_INFO("Search paths: {}", paths.size());
    for (auto path : paths)
    {
        SPDLOG_INFO("\t{}", path);
        s_paths.push_back(path);
    }
    s_paths.push_back("");

    SPDLOG_INFO("Filesystem initialized");
}

std::vector<uint8_t> Filesystem::Read(const std::string& path)
{
    SPDLOG_INFO("Reading file {}", path);

    for (auto& root : s_paths)
    {
        std::string fullPath = root + "/" + path;
        SPDLOG_DEBUG("Trying path {} for {}", fullPath, path);
        std::ifstream file(fullPath, std::ios::ate | std::ios::binary);
        if (file.is_open())
        {
            SPDLOG_INFO("Found file {} at {}", path,
                        fullPath);
            std::vector<uint8_t> data((size_t)file.tellg());
            file.seekg(std::ios::beg);
            file.read((char*)data.data(), data.size());
            SPDLOG_INFO("Read {} byte(s)", data.size());
            return data;
        }
    }

    return std::vector<uint8_t>();
}

bool Filesystem::Exists(const std::string& path)
{
    std::fstream file(path);
    bool exists = file.is_open();
    file.close();
    return exists;
}