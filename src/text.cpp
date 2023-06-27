#include "text.h"

static Image* s_font;
std::unordered_map<wchar_t, glm::u8vec2> s_characterPositions;
static bool s_initialized;

// FUCK UNICODE ARRRGGHH

void Text::Initialize()
{
    if (s_initialized)
    {
        return;
    }

    s_font = new Image("font.qoi");

    std::vector<uint8_t> fontTomlRaw = Filesystem::Read("font.toml");
    std::string fontToml(fontTomlRaw.begin(), fontTomlRaw.end());
    fontTomlRaw.clear();
    toml::parse_result fontDefinition = toml::parse(fontToml);
    if (fontDefinition.failed())
    {
        Quit(fmt::format("Failed to parse font definition: {}", fontDefinition.error().description()));
    }
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    for (const auto& character :
         (toml::table&)*fontDefinition["font"].as_table())
    {
        toml::array* position = character.second.as_array();
        if (!position || position->size() != 2 ||
            !position->at(0).is_integer() || !position->at(1).is_integer())
        {
            Quit(fmt::format("Invalid character definition \"{}\", expected "
                             "array of 2 integers",
                             character.first));
        }

        std::string chr(character.first);
        std::wstring wideChr = converter.from_bytes(chr);
        toml::value<int64_t> x = *position->at(0).as_integer();
        toml::value<int64_t> y = *position->at(1).as_integer();
        glm::u8vec2 pos((uint8_t)*x, (uint8_t)*y);
        s_characterPositions[wideChr[0]] = pos;
    }

    s_initialized = true;
}

void Text::Shutdown()
{
    if (!s_initialized)
    {
        return;
    }
    s_initialized = false;
    delete s_font;
}

void Text::DrawString(const std::string& text,
                      glm::uvec2 position, float scale, glm::u8vec3 color,
                      glm::uvec2 box, bool cutOff, glm::uvec2 padding)
{
    if (!s_initialized)
    {
        SPDLOG_WARN("DrawString called without initialization");
        return;
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wideText = converter.from_bytes(text);

    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t xSize = (uint32_t)((CHARACTER_SIZE + padding.x) * scale);
    uint32_t ySize = (uint32_t)((CHARACTER_SIZE + padding.y) * scale);
    for (wchar_t c : wideText)
    {
        if (box.y > 0 && cutOff && y * ySize > box.y)
        {
            break;
        }

        if (c == ' ' && x == 0)
        {
            continue;
        }

        switch (c)
        {
        case '\t': {
            x += 4;
            break;
        }
        case '\n': {
            x = 0;
            y++;
            break;
        }
        case '\r': {
            x = 0;
            break;
        }
        default: {
            if (c >= ' ')
            {
                g_backend->DrawImage(*s_font, x * xSize, y * ySize, scale, scale,
                                   s_characterPositions[c].x * CHARACTER_SIZE,
                                   s_characterPositions[c].y * CHARACTER_SIZE,
                                   CHARACTER_SIZE, CHARACTER_SIZE, color);
                x++;
            }
            break;
        }
        }

        if (box.x > 0 && x * xSize >= box.x)
        {
            x = 0;
            y++;
        }
    }
}
