#include "tool.h"

int32_t main(int32_t argc, char* argv[])
{
    std::vector<std::string> args(argc);
    for (int32_t i = 0; i < argc; i++)
    {
        args[i] = argv[i];
    }

    return ToolMain(args);
}

void Quit(const std::string& message, int32_t exitCode)
{
    if (exitCode == 1)
    {
        std::cerr << fmt::format("Fatal error: {}", message) << std::endl;
    }
    else
    {
        std::cerr << fmt::format("Fatal error {}: {}", exitCode, message) << std::endl;
    }

    exit(exitCode);
}
