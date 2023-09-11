#include "fs.h"
#include "vpk.h"

class PhysicalFileSource : public Core::Filesystem::FileSource
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

static std::vector<Core::Filesystem::FileSource*> s_fileSources;

CORE_API std::string Core::Filesystem::CleanPath(const std::string& path)
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

CORE_API void Core::Filesystem::Initialize(
    const std::vector<std::string>& searchPaths)
{
    SPDLOG_INFO("Initializing filesystem");

    for (const auto& path : searchPaths)
    {
        AddSearchPath(path);
    }

    // This might be a little wasteful but it's probably fine
    AddSearchPath("");

    SPDLOG_INFO("Filesystem initialized");
}

CORE_API void Core::Filesystem::AddSearchPath(const std::string& path)
{
    std::string cleanPath = CleanPath(path);
    SPDLOG_INFO("Adding search path {}", cleanPath);

    s_fileSources.push_back(FileSource::Create(cleanPath));
}

CORE_API Core::Filesystem::FileSource* Core::Filesystem::FileSource::Create(
    const std::string& path)
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

CORE_API std::vector<uint8_t> Core::Filesystem::Read(const std::string& path)
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

CORE_API void Core::Filesystem::Write(const std::string& path,
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
    size_t written = (size_t)file.tellp();
    if (written < data.size())
    {
        SPDLOG_WARN("Only wrote {}/{} byte(s)", written, data.size());
    }
    file.close();
}

CORE_API bool Core::Filesystem::Exists(const std::string& path)
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

CORE_API std::string Core::Filesystem::ResolvePath(const std::string& path)
{
    for (auto source : s_fileSources)
    {
        if (source->Exists(path))
        {
            return source->GetRealPath() + path;
        }
    }

    return path;
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
