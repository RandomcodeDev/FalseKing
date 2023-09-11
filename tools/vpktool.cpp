#include "core/fs.h"
#include "core/vpk.h"

#include "tool.h"

bool g_verbose;
bool g_debug;

// Initializes stuff like the log level
void Initialize()
{
    spdlog::set_level(spdlog::level::warn);
    if (g_verbose)
    {
        spdlog::set_level(spdlog::level::info);
    }
    if (g_debug)
    {
        spdlog::set_level(spdlog::level::trace);
    }
}

class CreateCommand : public Subcommand
{
  public:
    std::string inputDirectory;
    std::string outputVpk;

    void Run()
    {
        Initialize();

        fs::path directory = fs::absolute(inputDirectory);
        if (!outputVpk.length())
        {
            outputVpk = directory.string();
            if (outputVpk.back() == '\\' || outputVpk.back() == '/')
            {
                outputVpk.resize(outputVpk.length() - 1);
            }
            outputVpk = fmt::format("{}.vpk", outputVpk);
        }

        Core::Vpk::Vpk2 vpk(outputVpk, true);

        fmt::print("Creating VPK file {} from directory {}\n", outputVpk,
                   directory.string());
        for (const auto& entry : fs::recursive_directory_iterator(directory))
        {
            if (entry.is_regular_file() || entry.is_symlink())
            {
                std::string innerPath =
                    fs::relative(entry.path(), directory).string();
                vpk.AddFile(innerPath, Core::Filesystem::Read(entry.path().string()));
            }
        }

        vpk.Write();
    }
};

class ExtractCommand : public Subcommand
{
  public:
    std::string inputVpk;
    std::string outputDirectory;

    void Run()
    {
        Initialize();

        // 4 is length of _dir
        if (inputVpk.length() > 4 + Core::Vpk::VPK_EXTENSION_LENGTH &&
            inputVpk.substr(inputVpk.length() - 4 - Core::Vpk::VPK_EXTENSION_LENGTH,
                            4) == "_dir")
        {
            inputVpk.resize(inputVpk.length() - 4 - Core::Vpk::VPK_EXTENSION_LENGTH);
            inputVpk += Core::Vpk::VPK_EXTENSION;
        }
        fs::path vpkPath = fs::absolute(inputVpk);
        if (!outputDirectory.length())
        {
            outputDirectory = vpkPath.string();
            outputDirectory.resize(outputDirectory.length() -
                                   Core::Vpk::VPK_EXTENSION_LENGTH);
        }
        fs::path directoryPath(outputDirectory);

        fmt::print("Extracting VPK file {} to directory {}\n", vpkPath.string(),
                   directoryPath.string());

        Core::Vpk::Vpk2 vpk(vpkPath.string());
        for (const auto& entry : vpk)
        {
            fs::path path(entry.first);
            path = directoryPath / path;
            fs::create_directories(path.parent_path());
            if (g_verbose)
            {
                fmt::print("Writing file {}\n", path.string());
            }
            Core::Filesystem::Write(path.string(), vpk.Read(entry.first));
        }
    }
};

class ListCommand : public Subcommand
{
  public:
    std::string inputVpk;
    bool checkHashes;

    void Run()
    {
        Initialize();

        // 4 is length of _dir
        if (inputVpk.length() > 4 + Core::Vpk::VPK_EXTENSION_LENGTH &&
            inputVpk.substr(inputVpk.length() - 4 - Core::Vpk::VPK_EXTENSION_LENGTH,
                            4) == "_dir")
        {
            inputVpk.resize(inputVpk.length() - 4 - Core::Vpk::VPK_EXTENSION_LENGTH);
            inputVpk += Core::Vpk::VPK_EXTENSION;
        }
        fs::path vpkPath = fs::absolute(inputVpk);

        Core::Vpk::Vpk2 vpk(vpkPath.string());
        const auto& header = vpk.GetHeader();
        fmt::print("Header:\n");
        fmt::print("\tSignature: {:08X} (should be {:08X})\n", header.signature,
                   Core::Vpk::VPK2_SIGNATURE);
        fmt::print("\tVersion: {}\n", header.version);
        fmt::print("\tTree size: {}\n", header.treeSize);
        fmt::print("\tSize of file data in directory: {}\n",
                   header.fileDataSize);
        fmt::print("\tMD5 section size: {}\n", header.externalMd5Size);
        fmt::print("\tInternal MD5 data size: {}\n", header.md5Size);
        fmt::print("\tSignature data size: {}\n", header.signature);
        fmt::print("\tFile count: {}\n", vpk.GetFileCount());
        for (const auto& pair : vpk)
        {
            const auto& name = pair.first;
            const auto& entry = pair.second;

            fmt::print("Entry {}\n", name);
            if (checkHashes)
            {
                uint32_t crc = vpk.ComputeCrc32(name);
                fmt::print("\tStored CRC: 0x{:08X} (computed 0x{:08X}{})\n",
                           entry.crc, crc,
                           entry.crc != crc ? ", data could be incorrect"
                                            : ", data is correct");
            }
            else
            {
                fmt::print("\tCRC: 0x{:08X}\n", entry.crc);
            }
            fmt::print("\tPreload size: {}\n", entry.preloadSize);
            fmt::print("\tArchive index: {}\n", entry.archiveIndex);
            fmt::print("\tOffset: 0x{:08X}\n", entry.offset);
            fmt::print("\tLength: {}\n", entry.length);
        }
    }
};

int32_t ToolMain()
{
    CLI::App app(fmt::format("{} Valve Pack File (VPK) Tool", Core::GAME_NAME));
    app.set_version_flag("--version",
                         fmt::format("v{}.{}.{}", Core::GAME_MAJOR_VERSION,
                                     Core::GAME_MINOR_VERSION, Core::GAME_PATCH_VERSION));
    app.set_help_all_flag("--help-all", "Extra help");
    app.require_subcommand();

    app.add_flag<bool>("-v,--verbose", g_verbose,
                       "Output information for each file");
    app.add_flag<bool>("-d,--debug", g_debug, "Output debugging information");

    auto create =
        app.add_subcommand("create", "Create a VPK file from a directory");
    CreateCommand createData{};
    create
        ->add_option("-i,--directory", createData.inputDirectory,
                     "The directory to make a VPK file from")
        ->type_name("FILE")
        ->required();
    create
        ->add_option("-o,--vpk", createData.outputVpk,
                     "The path to save the VPK file to (defaults to the name "
                     "of the directory)")
        ->type_name("DIR");
    createData.Register(create);

    auto extract =
        app.add_subcommand("extract", "Extract a VPK file to a directory");
    ExtractCommand extractData{};
    extract
        ->add_option("-i,--vpk", extractData.inputVpk,
                     "The VPK file to extract")
        ->type_name("FILE")
        ->required();
    extract
        ->add_option("-o,--directory", extractData.outputDirectory,
                     "The directory to extract the VPK file into (will be "
                     "created, defaults to the name of the VPK)")
        ->type_name("DIR");
    extractData.Register(extract);

    auto list = app.add_subcommand(
        "list", "List the information and contents of a VPK file");
    ListCommand listData{};
    list->add_option("-i,--vpk", listData.inputVpk, "The VPK file to read")
        ->type_name("FILE")
        ->required();
    list->add_flag("-c,--check", listData.checkHashes,
                   "Check the hashes of files where possible");
    listData.Register(list);

    Core::Filesystem::Initialize();

    CLI11_PARSE(app);

    return 0;
}