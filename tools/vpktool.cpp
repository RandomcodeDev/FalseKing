#include "fs.h"
#include "tool.h"
#include "vpk2.h"

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
        spdlog::set_level(spdlog::level::debug);
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

        Vpk::Vpk2 vpk(outputVpk, true);

        SPDLOG_INFO("Creating VPK file {} from directory {}", outputVpk,
                    directory.string());
        for (const auto& entry : fs::recursive_directory_iterator(directory))
        {
            if (entry.is_regular_file() || entry.is_symlink())
            {
                std::string innerPath =
                    fs::relative(entry.path(), directory)
                        .string();
                vpk.AddFile(innerPath, Filesystem::Read(entry.path().string()));
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
        if (inputVpk.length() > 4 + Vpk::VPK_EXTENSION_LENGTH &&
            inputVpk.substr(inputVpk.length() - 4 - Vpk::VPK_EXTENSION_LENGTH,
                            4) == "_dir")
        {
            inputVpk.resize(inputVpk.length() - 4 - Vpk::VPK_EXTENSION_LENGTH);
            inputVpk += Vpk::VPK_EXTENSION;
        }
        fs::path vpkPath = fs::absolute(inputVpk);
        if (!outputDirectory.length())
        {
            outputDirectory = vpkPath.string();
            outputDirectory.resize(outputDirectory.length() -
                                   Vpk::VPK_EXTENSION_LENGTH);
        }
        fs::path directoryPath(outputDirectory);

        SPDLOG_INFO("Extracting VPK file {} to directory {}", vpkPath.string(),
                    directoryPath.string());

        Vpk::Vpk2 vpk(vpkPath.string());
        for (const auto& entry : vpk)
        {
            fs::path path(entry.first);
            path = directoryPath / path;
            fs::create_directories(path.parent_path());
            SPDLOG_INFO("Writing file {}", path.string());
            Filesystem::Write(path.string(), vpk.Read(entry.first));
        }
    }
};

int32_t ToolMain()
{
    CLI::App app(fmt::format("{} Valve Pack File (VPK) Tool", GAME_NAME));
    app.set_version_flag("--version",
                         fmt::format("v{}.{}.{}", GAME_MAJOR_VERSION,
                                     GAME_MINOR_VERSION, GAME_PATCH_VERSION));
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
    create->callback([&createData]() { createData.Run(); });

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
    extract->callback([&extractData]() { extractData.Run(); });

    Filesystem::Initialize();

    CLI11_PARSE(app);

    return 0;
}