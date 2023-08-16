workspace 'Game'

    default_workspace_settings()

project 'Game'

    default_project_settings()

    kind 'WindowedApp'

    files {
        'include/**.h',
        'src/**.cpp',
    }

    removefiles {
        'src/sdl.cpp',
    }

    filter { 'files:src/**.cpp' }
        pchheader 'stdafx.h'
        pchsource 'stdafx.cpp'
    filter {}
    filter { 'files:stdafx.cpp' }
        includedirs {
            _MAIN_SCRIPT_DIR
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett' }
        files {
            'gdk/**'
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or macosx or linux' }
        files {
            -- Get Discord and ImGui files
            'deps-public/src/**',

            'src/sdl.cpp'
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

    filter { 'system:not macosx', 'architecture:not x86' }
        links {
            'PhysX_static_64',
            'PhysXCharacterKinematic_static_64',
            'PhysXCommon_static_64',
            'PhysXExtensions_static_64',
            'PhysXFoundation_static_64',
            'PhysXPvdSDK_static_64'
        }
    filter { 'system:not macosx', 'architecture:x86' }
        links {
            'PhysX_static_32',
            'PhysXCharacterKinematic_static_32',
            'PhysXCommon_static_32',
            'PhysXExtensions_static_32',
            'PhysXFoundation_static_32',
            'PhysXPvdSDK_static_32'
        }
    filter { 'system:macosx' }
        links {
            'PhysX-darwin',
            'PhysXCharacterKinematic-darwin',
            'PhysXCommon-darwin',
            'PhysXExtensions-darwin',
            'PhysXFoundation-darwin',
            'PhysXPvdSDK-darwin'
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or linux' }
        prebuildcommands {
            'python %{wks.location}/../scripts/commit.py %{cfg.targetdir}'
        }
        prelinkcommands {
            'python %{wks.location}/../scripts/copyfiles.py %{cfg.targetdir} %{cfg.system} %{cfg.platform} %{cfg.architecture} %{cfg.buildcfg} build'
        }
    filter { 'system:macosx' }
        prebuildcommands {
            '%{wks.location}/../scripts/commit.py %{cfg.targetdir}'
        }
        prelinkcommands {
            '%{wks.location}/../scripts/copyfiles.py %{cfg.targetdir} %{cfg.system} Universal Universal %{cfg.buildcfg} build'
        }
    filter {}