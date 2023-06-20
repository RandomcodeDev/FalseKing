#include "backend.h"
#include "fs.h"
#include "game.h"
#include "input.h"
#include "text.h"

extern void TestText(Backend* backend, InputState& input);

int GameMain(Backend* backend, std::vector<fs::path> backendPaths)
{
    Filesystem::Initialize(backendPaths);
    Text::Initialize();

    InputState input;

    TestText(input);

    Text::Shutdown();

    return 0;
}
