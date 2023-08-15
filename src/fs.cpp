#include "fs.h"
#include "vpk2.h"

class PhysicalFileSource : public Filesystem::FileSource
{
  public:
    PhysicalFileSource(const std::string& path);
    ~PhysicalFileSource() = default;

    const std::string& GetRealPath()
    {
        return m_realPath;
    }

    std::vector<uint8_t> Read(const std::string& path);
    bool Exists(const std::string& path);

  private:
    std::string m_realPath;
};

static std::vector<Filesystem::FileSource*> s_fileSources;

std::string Filesystem::CleanPath(const std::string& path)
{
    std::string cleanPath = path;
    size_t i = 0;
    std::replace(cleanPath.begin(), cleanPath.end(), '\\', '/');
    if (i > 0 && cleanPath[i - 1] == '/')
    {
        cleanPath.resize(cleanPath.size() - 1);
    }

    return cleanPath;
}

void Filesystem::Initialize(const std::vector<std::string>& searchPaths)
{
    SPDLOG_INFO("Initializing filesystem");

    for (auto path : searchPaths)
    {
        AddSearchPath(path);
    }
    AddSearchPath("");

    SPDLOG_INFO("Filesystem initialized");
}

void Filesystem::AddSearchPath(const std::string& path)
{
    std::string cleanPath = CleanPath(path);
    SPDLOG_INFO("Adding search path {}", cleanPath);

    s_fileSources.push_back(FileSource::Create(cleanPath));
}

Filesystem::FileSource* Filesystem::FileSource::Create(const std::string& path)
{
    if (path.length() > 4 && path.substr(path.length() - 4, path.length()) == ".vpk")
    {
        return new Vpk::Vpk2(path);
    }
    else
    {
        return new PhysicalFileSource(path);
    }
}

std::vector<uint8_t> Filesystem::Read(const std::string& path)
{
    SPDLOG_INFO("Reading file {}", path);

    for (auto source : s_fileSources)
    {
        if (source->Exists(path))
        {
            return source->Read(path);
        }
    }

    return std::vector<uint8_t>();
}

void Filesystem::Write(const std::string& path,
                       const std::vector<uint8_t>& data)
{
    SPDLOG_INFO("Writing {} byte(s) to {}", data.size(), path);
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        SPDLOG_WARN("Failed to open file {}", path);
        return;
    }
    file.write((const char*)data.data(), data.size());
    size_t written = file.tellp();
    if (written < data.size())
    {
        SPDLOG_WARN("Only wrote {}/{} byte(s)", written, data.size());
    }
    file.close();
}

bool Filesystem::Exists(const std::string& path)
{
    for (auto source : s_fileSources)
    {
        if (source->Exists(path))
        {
            return true;
        }
    }

    return false;
}

PhysicalFileSource::PhysicalFileSource(const std::string& path)
{
    m_realPath = path;
}

std::vector<uint8_t> PhysicalFileSource::Read(const std::string& path)
{
    std::string fullPath = m_realPath + (m_realPath.length() ? "/" : "") + path;
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

    return std::vector<uint8_t>();
}

bool PhysicalFileSource::Exists(const std::string& path)
{
    std::string fullPath = m_realPath + (m_realPath.length() ? "/" : "") + path;
    std::fstream file(fullPath);
    bool exists = file.is_open();
    file.close();
    return exists;
}
