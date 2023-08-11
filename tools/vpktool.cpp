#include "game.h"
#include "fs.h"
#include "vpk2.h"

int32_t ToolMain(const std::vector<std::string>& args)
{
    spdlog::set_level(spdlog::level::debug);

    Filesystem::Initialize();

    Vpk::Vpk2 vpk("test.vpk", true);
    vpk.AddFile("vpktool-vs2022.vcxproj", Filesystem::Read("vpktool-vs2022.vcxproj"));

    return 0;
}