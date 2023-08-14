#include "tool.h"

int32_t main()
{
    return ToolMain();
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
