require 'premake-consoles/consoles'

workspace 'Game'

    location 'build'

    configurations { 'Debug', 'Release' }

    filter { 'system:windows' }
        platforms { 'x86', 'ARM64' }
    filter { 'system:gaming_desktop' }
        platforms { 'Gaming.Desktop.x64' }
    filter { 'system:scarlett' }
        platforms { 'Gaming.Xbox.Scarlett.x64' }
    filter { 'system:linux' }
        platforms { 'x86_64', 'ARM64' }
    filter {}
        
    filter { 'toolset:msc' }
        toolset 'v141_xp'
    filter {}

    filter { 'platforms:ARM64' }
        architecture 'arm64'
    filter {}

project 'Game'

    location 'build/Game'

    targetname 'Game.%{cfg.platform}'

    language 'C++'
    cppdialect 'C++17'

    objdir '%{prj.location}/%{cfg.platform}/%{cfg.buildcfg}'
    targetdir '%{wks.location}/%{cfg.platform}/%{cfg.buildcfg}'

    kind 'WindowedApp'

    files {
        '.github/**',
        'include/**.h',
        'src/**.cpp',
        'scripts/*',
        '.clang-format',
        '.gitattributes',
        '.gitignore',
        '.gitmoduls',
        'DESIGN.md',
        'LICENSE.txt',
        'logo.png',
        'premake5.lua',
        'README.md',
    }

    removefiles {
        'src/sdl.cpp',
    }

    filter { 'system:gaming_desktop or scarlett' }
        files {
            'gdk/**'
        }
    filter {}

    includedirs {
        'include',
        '%{cfg.targetdir}'
    }

    filter { 'architecture:not x86' }
        links {
            'PhysX_static_64',
            'PhysXCharacterKinematic_static_64',
            'PhysXCommon_static_64',
            'PhysXExtensions_static_64',
            'PhysXFoundation_static_64',
            'PhysXPvdSDK_static_64'
        }
    filter { 'architecture:x86' }
        links {
            'PhysX_static_32',
            'PhysXCharacterKinematic_static_32',
            'PhysXCommon_static_32',
            'PhysXExtensions_static_32',
            'PhysXFoundation_static_32',
            'PhysXPvdSDK_static_32'
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or macosx or linux' }
        files {
            'deps-public/include/**',
            'deps-public/src/**',
            'src/sdl.cpp'
        }

        defines {
            'USE_SDL'
        }

        includedirs {
            'deps-public/include',
        }

        libdirs {
            'deps-public/lib/%{cfg.architecture}/%{cfg.buildcfg}',
            'deps-public/lib/%{cfg.architecture}'
        }

        links {
            'SDL3'
        }
    filter {}

    defines {
        'NOMINMAX',
        '_CRT_SECURE_NO_DEPRECATE',
        '_CRT_SECURE_NO_WARNINGS',
    }

    flags { 'MultiProcessorCompile' }
    staticruntime 'Off'
    
    filter { 'configurations:Debug' }
        defines '_DEBUG'
        symbols 'On'
    filter { 'configurations:Release' }
        defines 'NDEBUG'
        optimize 'On'
    filter {}

    filter { 'system:linux' }
        -- PhysXPvdSDK wants PhysXGpu without partial linking, which GNU ld doesn't do by default
        linkoptions { '-r' }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows' }
        prebuildcommands {
            '%{wks.location}/../scripts/commit.bat %{cfg.targetdir}'
        }
        prelinkcommands {
            '%{wks.location}/../scripts/copyfiles.bat %{cfg.targetdir} %{cfg.platform} %{cfg.buildcfg}'
        }
    filter { 'system:macosx or linux' }
        prebuildcommands {
            '%{wks.location}/../scripts/commit.sh %{cfg.targetdir}'
        }
        prelinkcommands {
            '%{wks.location}/../scripts/copyfiles.sh %{cfg.targetdir} %{cfg.platform} %{cfg.buildcfg}'
        }
    filter {}
