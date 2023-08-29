workspace 'Game'

    default_workspace_settings()

project 'Launcher'

    default_project_settings()

    kind 'WindowedApp'

    files {
        'launcher/**.h',
        'launcher/**.cpp'
    }

    filter { 'files:core/**.cpp' }
        pchheader 'stdafx.h'
        pchsource 'stdafx.cpp'
    filter {}
    filter { 'files:stdafx.cpp' }
        includedirs {
            _MAIN_SCRIPT_DIR
        }
    filter {}

    links {
        'Core',
        'Game'
    }

    filter { 'system:gaming_desktop or scarlett' }
        files {
            'gdk/**'
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or macosx or linux' }
        files {
            'launcher/sdl.cpp'
        }

        defines {
            'USE_SDL'
        }
    filter { 'system:gaming_desktop or scarlett or windows or linux' }
        links {
            'SDL3'
        }
    filter { 'system:macosx' }
        links {
            'SDL3.framework'
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or linux' }
        prebuildcommands {
            'python3 %{wks.location}/../scripts/commit.py %{cfg.targetdir}'
        }
        prelinkcommands {
            'python3 %{wks.location}/../scripts/copyfiles.py --system %{cfg.system} --platform %{cfg.platform} --architecture %{cfg.architecture} --configuration %{cfg.buildcfg} %{cfg.targetdir} build'
        }
    filter { 'system:macosx' }
        prebuildcommands {
            '%{wks.location}/../scripts/commit.py %{cfg.targetdir}'
        }
        prelinkcommands {
            'python3 %{wks.location}/../scripts/copyfiles.py --system %{cfg.system} --configuration %{cfg.buildcfg} %{cfg.targetdir} build'
        }
    filter {}

include 'core.lua'

project 'Game'

    default_project_settings()

    kind 'SharedLib'

    files {
        'game/**.h',
        'game/**.cpp'
    }

    defines {
        'GAME=1'
    }

    links {
        'Core'
    }

    filter { 'files:game/**.cpp' }
        pchheader 'stdafx.h'
        pchsource 'stdafx.cpp'
    filter {}
    filter { 'files:core/stdafx.cpp' }
        includedirs {
            _MAIN_SCRIPT_DIR
        }
    filter {}
