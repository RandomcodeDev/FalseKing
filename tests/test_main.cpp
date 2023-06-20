#include "backend.h"
#include "fs.h"
#include "game.h"
#include "input.h"
#include "text.h"

Backend* g_backend;

extern void TestText(InputState& input);

int GameMain(Backend* backend, std::vector<fs::path> backendPaths)
{
    g_backend = backend;
    Filesystem::Initialize(backendPaths);
    Text::Initialize();

    InputState input;

    TestText(input);

    Text::Shutdown();

    return 0;
}
