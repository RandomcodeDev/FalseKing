#include "backend.h"
#include "fs.h"
#include "game.h"
#include "input.h"
#include "text.h"

void TestText(InputState& input)
{
    std::vector<uint8_t> textData = Filesystem::Read("communism.txt");
    std::string text(textData.begin(), textData.end());
    float zoom = 0.5f;
    float scrolling = 0.0f;

    bool run = true;
    chrono::time_point<precise_clock> start = precise_clock::now();
    chrono::time_point<precise_clock> now;
    chrono::time_point<precise_clock> last;
    while (run)
    {

        now = precise_clock::now();
        chrono::milliseconds delta =
            chrono::duration_cast<chrono::milliseconds>(now - last);
        float floatDelta =
            (float)delta.count() / chrono::milliseconds::period::den;

        if (!g_backend->Update(input) || input.GetStart())
        {
            break;
        }

        if (!g_backend->BeginRender())
        {
            continue;
        }

        zoom += input.GetScrollAmount() / 2;
        input.ResetScrollAmount();

        Text::DrawString(text, glm::uvec2(0), zoom, glm::u8vec3(0, 255, 0));

        g_backend->EndRender();

        last = now;
        float ratio = 1.0f / 60.0f;
        if (floatDelta < ratio)
        {
            std::this_thread::sleep_for(chrono::milliseconds(
                (uint32_t)((ratio - floatDelta) * 1000.0f)));
        }
    }
}