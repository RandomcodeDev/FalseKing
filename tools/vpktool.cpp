#include "fs.h"
#include "tool.h"
#include "vpk2.h"

bool g_verbose;
bool g_silent;

// Initializes stuff like the log level
void Initialize()
{
    if (g_verbose && !g_silent)
    {
        spdlog::set_level(spdlog::level::debug);
    }
    else if (!g_verbose && g_silent)
    {
        spdlog::set_level(spdlog::level::warn);
    }
    else
    {
        spdlog::set_level(spdlog::level::info);
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
                    fs::relative(entry.path(), directory.parent_path())
                        .string();
                SPDLOG_DEBUG("{} -> {}", entry.path().string(), innerPath);
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
                       "Output debugging information");
    app.add_flag<bool>("-s,--silent", g_silent, "Output reduced information");

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