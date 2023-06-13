#include "fs.h"
#include "game.h"

[[noreturn]]
void Usage(const std::string& name);

void AddFile(PackTable& pack, const fs::path& path);

void AddDirectory(PackTable& pack, const fs::path& path);

int main(int argc, char* argv[])
{
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    if (argc < 3)
    {
        Usage(argv[0]);
    }

    PackTable pack(argv[1], true);
    std::vector<fs::path> paths(argv + 2, argv + argc);
    for (const auto& path : paths)
    {
        if (fs::is_directory(path))
        {
            AddDirectory(pack, path);
        }
        else
        {
            AddFile(pack, path);
        }
    }

    pack.Save();
    SPDLOG_INFO("Done adding to pack file");
}

void AddFile(PackTable& pack, const fs::path& path)
{
    SPDLOG_DEBUG("Adding file {}", path.string());
    std::ifstream file(path, std::ios::ate);
    if (!file.good())
    {
        SPDLOG_WARN("File {} could not be opened", path.string());
        return;
    }
    size_t size = file.tellg();
    std::vector<uint8_t> data(size);
    file.seekg(std::ios::beg);
    file.read((char*)data.data(), data.size());
    std::string name = fs::relative(path).string();
    pack.Update(name, data);
}

void AddDirectory(PackTable& pack, const fs::path& path)
{
    for (const auto& entry : fs::recursive_directory_iterator(path))
    {
        if (!fs::is_directory(entry.path()))
        {
            AddFile(pack, path);
        }
    }
}

[[noreturn]]
void Quit(const std::string& message, int exitCode)
{
    SPDLOG_ERROR("Error {0}/0x{0:X}: {1}", exitCode, message);
    exit(exitCode);
}

[[noreturn]]
void Usage(const std::string& name)
{
    SPDLOG_INFO("Usage: {} <pack file> <file 1> [file 2] [file 3] ...", name);
    exit(1);
}
