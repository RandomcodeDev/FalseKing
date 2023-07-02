#include "fs.h"

static std::vector<std::string> s_paths;

void Filesystem::Initialize(const std::vector<std::string>& paths)
{
    SPDLOG_INFO("Initializing filesystem");

    SPDLOG_INFO("Search paths: {}", paths.size());
    for (auto path : paths)
    {
        std::string cleanPath = path;
        size_t i = 0;
        for (; i < cleanPath.size(); i++)
        {
            if (cleanPath[i] == '\\')
            {
                cleanPath[i] = '/';
            }
        }
        if (cleanPath[i - 1] == '/')
        {
            cleanPath.resize(cleanPath.size() - 1);
        }
        SPDLOG_INFO("\t{}", cleanPath);
        s_paths.push_back(cleanPath);
    }
    s_paths.push_back("");

    SPDLOG_INFO("Filesystem initialized");
}

std::vector<uint8_t> Filesystem::Read(const std::string& path)
{
    SPDLOG_INFO("Reading file {}", path);

    for (auto& root : s_paths)
    {
        std::string fullPath = root + (root.length() ? "/" : "") + path;
        SPDLOG_DEBUG("Trying path {} for {}", fullPath, path);
        std::ifstream file(fullPath, std::ios::ate | std::ios::binary);
        if (file.is_open())
        {
            SPDLOG_INFO("Found file {} at {}", path, fullPath);
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