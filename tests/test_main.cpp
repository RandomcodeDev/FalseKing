#define QOI_IMPLEMENTATION

#include "backend.h"
#include "fs.h"
#include "game.h"
#include "input.h"
#include "text.h"

extern void TestText(Backend* backend, InputState& input);

int GameMain(Backend* backend, std::vector<fs::path> backendPaths)
{
    Filesystem::Initialize(backendPaths);
    Text::Initialize(backend);

    InputState input;

    TestText(backend, input);

    Text::Shutdown(backend);

    return 0;
}
