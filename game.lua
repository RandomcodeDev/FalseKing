workspace 'Game'

    default_workspace_settings()

    startproject 'Launcher'

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

    filter { 'files:gdk/MicrosoftGameConfig.mgc' }
        buildaction 'MGCCompile'
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or macosx or linux or psp' }
        files {
            'launcher/sdl.cpp'
        }

        defines {
            'USE_SDL'
        }
    filter { 'system:psp' }
        links {
            'GL',
            'PhysX_static',
            'PhysXCharacterKinematic_static',
            'PhysXCommon_static',
            'PhysXExtensions_static',
            'PhysXFoundation_static',
            'PhysXPvdSDK_static',
            'pspvram',
            'pspaudio',
            'pspvfpu',
            'pspdisplay',
            'pspgu',
            'pspge',
            'psphprm',
            'pspctrl',
            'psppower',
            'cglue',
            'atomic',
        }
    filter { 'system:gaming_desktop or scarlett or windows or linux or psp' }
        links {
            'SDL3'
        }
    filter { 'system:macosx' }
        links {
            'SDL3.framework'
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or linux or psp' }
        prelinkcommands {
            'python3 %{wks.location}/../scripts/copyfiles.py --system %{cfg.system} --platform %{cfg.platform} --architecture %{cfg.architecture} --configuration %{cfg.buildcfg} %{cfg.targetdir} build'
        }
    filter { 'system:macosx' }
        prelinkcommands {
            'python3 %{wks.location}/../scripts/copyfiles.py --system %{cfg.system} --configuration %{cfg.buildcfg} %{cfg.targetdir} build'
        }
    filter {}

isTools = nil
dofile 'core.lua'

project 'Game'

    default_project_settings()

    filter { 'system:psp' }
        kind 'StaticLib'
    filter { 'system:not psp' }
        kind 'SharedLib'
    filter {}

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
