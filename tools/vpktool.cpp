#include "game.h"
#include "fs.h"
#include "vpk2.h"

int32_t ToolMain(const std::vector<std::string>& args)
{
    spdlog::set_level(spdlog::level::debug);

    Filesystem::Initialize();

    Vpk::Vpk2 vpk("test.vpk", false);
    SPDLOG_INFO("vpktool-vs2022.vcxproj: {}",
                (const char*)vpk.Read("vpktool-vs2022.vcxproj").data());
    //vpk.AddFile("vpktool-vs2022.vcxproj", Filesystem::Read("vpktool-vs2022.vcxproj"));
    //vpk.AddFile("vpk2ool-vs2022.vcxproj.filters",
                //Filesystem::Read("vpktool-vs2022.vcxproj.filters"));
    //vpk.Write();

    return 0;
}