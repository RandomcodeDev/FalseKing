#include "backend.h"
#include "fs.h"
#include "game.h"
#include "input.h"
#include "text.h"

void TestText(Backend* backend, InputState& input)
{
    std::vector<uint8_t> textData = Filesystem::Read("communism.txt");
    std::string text(textData.begin(), textData.end());
    float zoom = 0.3f;

    while (true)
    {
        if (!backend->Update(input) || input.GetStart())
        {
            break;
        }

        if (!backend->BeginRender())
        {
            continue;
        }

        if (input.GetLeftShoulder())
        {
            zoom += 0.05;
        }
        else if (input.GetRightShoulder())
        {
            zoom -= 0.05;
        }

        Text::DrawString(backend, text, glm::uvec2(0), zoom);

        backend->EndRender();
    }
}
