#include "discord.h"
#include "backend.h"

#ifndef DISCORD_ENABLE
#if (defined(_WIN32) && !defined(WINAPI_FAMILY_WINRT)) ||      \
    defined(__APPLE__) || defined(__linux__)
#define DISCORD_ENABLE 1
#else
#define DISCORD_ENABLE 0
#endif
#endif

static discord::Core* core{};
static bool connected;
static bool available;
static chrono::milliseconds requestCooldown;
static chrono::milliseconds activityCooldown;

static std::unordered_map<discord::LogLevel, spdlog::level::level_enum>
    levelMap;
static std::vector<uint64_t> friends;

static void* discordSdkDll;
typedef enum EDiscordResult(DISCORD_API* DiscordCreatePtr)(
    DiscordVersion version, struct DiscordCreateParams* params,
    struct IDiscordCore** result);
static DiscordCreatePtr DiscordCreate_loaded;

extern "C" enum EDiscordResult DISCORD_API
DiscordCreate(DiscordVersion version, struct DiscordCreateParams* params,
              struct IDiscordCore** result)
{
    if (!discordSdkDll)
    {
        SPDLOG_INFO("Loading Discord SDK");
        discordSdkDll = g_backend->LoadLibrary("discord_game_sdk");
        if (!discordSdkDll)
        {
            SPDLOG_ERROR("Failed to load Discord SDK");
            available = false;
            return DiscordResult_InvalidVersion;
        }

        DiscordCreate_loaded = (DiscordCreatePtr)g_backend->GetSymbol(
            discordSdkDll, "DiscordCreate");
        if (!DiscordCreate_loaded)
        {
            SPDLOG_ERROR("Failed to get symbol DiscordCreate");
            available = false;
            return DiscordResult_InvalidVersion;
        }

        available = true;
        SPDLOG_INFO("Discord SDK loaded");
    }

    return DiscordCreate_loaded(version, params, result);
}

void Discord::Initialize()
{
#if DISCORD_ENABLE
    SPDLOG_INFO("Connecting to Discord");

    discord::Result result =
        discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core);
    if (result != discord::Result::Ok)
    {
        SPDLOG_ERROR("Couldn't connect to Discord: {}", (uint32_t)result);
        connected = false;
        return;
    }

    levelMap[discord::LogLevel::Debug] = spdlog::level::debug;
    levelMap[discord::LogLevel::Error] = spdlog::level::err;
    levelMap[discord::LogLevel::Info] = spdlog::level::info;
    levelMap[discord::LogLevel::Warn] = spdlog::level::warn;

    friends.push_back(526963318867492865);
    friends.push_back(744607381991718913);
    friends.push_back(887865846414868521);
    friends.push_back(898953887988482068);
    friends.push_back(570760243341033472);
    friends.push_back(551486661079334912);
    friends.push_back(802941540120789014);
    friends.push_back(436582998171844608);
    friends.push_back(344253142952574989);
    friends.push_back(522809695392497664);
    friends.push_back(621474796500418570);
    friends.push_back(691017722934329394);
    friends.push_back(515919551444025407);
    friends.push_back(273987143523762176);
    friends.push_back(733739521438646342);
    friends.push_back(642135253615640598);
    friends.push_back(464268944459563018);
    friends.push_back(343862296751112192);
    friends.push_back(664575599737569303);

    core->SetLogHook(discord::LogLevel::Debug,
                     [](discord::LogLevel level, const char* message) {
                         SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(),
                                            levelMap[level], message);
                     });
    core->UserManager().OnCurrentUserUpdate.Connect([]() {
        discord::User user;
        core->UserManager().GetCurrentUser(&user);
        SPDLOG_INFO("Discord user changed to {}{} with ID {}",
                    user.GetBot() ? "bot " : "", user.GetUsername(),
                    user.GetId());
    });

    connected = true;
    requestCooldown = chrono::milliseconds(0);
    activityCooldown = chrono::milliseconds(0);
    SPDLOG_INFO("Discord connected");
#else
    SPDLOG_INFO("Discord disabled");
    connected = false;
#endif
}

#if DISCORD_ENABLE
static bool IsMyFriend(discord::UserId id)
{
    return std::find(friends.begin(), friends.end(), (uint64_t)id) !=
           friends.end();
}

static const char* GetCoolString()
{
    if (!connected)
    {
        return "game";
    }

    discord::User user;
    core->UserManager().GetCurrentUser(&user);
    discord::UserId id = user.GetId();
    
    switch (id)
    {
    case 532320702611587112:
        return "my game";
    case 1078816552629051423:
        return "my brother's game";
    case 405454975750373376:
        return "the game I came up with for my friend";
    default:
        if (IsMyFriend(id))
        {
            return "my friend's game";
        }
        else
        {
            return "game";
        }
    }
}
#endif

void Discord::Update(chrono::seconds runtime, chrono::milliseconds delta)
{
#if DISCORD_ENABLE
    if (!connected)
    {
        return;
    }

    discord::Activity activity{};

    std::string state =
        fmt::format("Playing {} for {:%T}", GetCoolString(), runtime);
    activity.SetState(state.c_str());
    std::string details = fmt::format(
        "{} {}.{}.{}\nCommit {:.7}\nRunning {} build\n{}", GAME_NAME,
        GAME_MAJOR_VERSION, GAME_MINOR_VERSION, GAME_PATCH_VERSION, GAME_COMMIT,
#if _WIN32_WINNT == _WIN32_WINNT_WINXP
        "Windows",
#elif defined(_WIN32)
        "Universal Windows",
#elif defined(__APPLE__)
        "macOS",
#elif defined(__linux__)
        "Linux",
#else
        "unknown",
#endif
        g_backend->DescribeSystem());
    activity.SetDetails(details.c_str());
    activity.SetType(discord::ActivityType::Playing);

    requestCooldown -= delta;
    activityCooldown -= delta;

    if (activityCooldown.count() <= 0)
    {
        core->ActivityManager().UpdateActivity(
            activity, [](discord::Result result) {
                if (result != discord::Result::Ok)
                {
                    SPDLOG_WARN("Failed to update activity: {}",
                                (uint32_t)result);
                }
            });
    }

    if (requestCooldown.count() <= 0)
    {
        core->RunCallbacks();
        requestCooldown = API_COOLDOWN;
    }
#endif
}

void Discord::Shutdown()
{
#if DISCORD_ENABLE
    if (!connected)
    {
        return;
    }

    SPDLOG_INFO("Disconnecting Discord");
    connected = false;
    friends.clear();
    levelMap.clear();
    delete core;
    SPDLOG_INFO("Discord disconnected");
#endif
}

bool Discord::Available()
{
    return available;
}

bool Discord::Connected()
{
    return connected;
}
