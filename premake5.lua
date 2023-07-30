require 'premake-consoles/consoles'

local OSEXT=''
if os.target() == 'macosx' then
    OSEXT='-darwin'
end

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
        
    filter { 'system:windows', 'platforms:x86' }
        toolset 'msc-v141_xp'
    filter {}

    filter { 'platforms:ARM64' }
        architecture 'arm64'
    filter {}

project 'Game'

    location 'build/Game'

    targetname 'Game.%{cfg.platform}'

    language 'C++'
    cppdialect 'C++17'

    filter { 'system:not macosx' }
        objdir '%{prj.location}/%{cfg.platform}/%{cfg.buildcfg}'
        targetdir '%{wks.location}/%{cfg.platform}/%{cfg.buildcfg}'
    filter { 'system:macosx' }
        objdir '%{prj.location}/Universal/%{cfg.buildcfg}'
        targetdir '%{wks.location}/Universal/%{cfg.buildcfg}'
    filter {}

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

        frameworkdirs {
            'deps-public/Frameworks/%{cfg.buildcfg}',
            'deps-public/Frameworks'
        }

        libdirs {
            'deps-public/lib/%{cfg.architecture}/%{cfg.buildcfg}',
            'deps-public/lib/%{cfg.architecture}',
            'deps-public/lib/Universal/%{cfg.buildcfg}',
            'deps-public/lib/Universal'
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

    filter { 'architecture:not x86' }
        links {
            'PhysX_static_64' .. OSEXT,
            'PhysXCharacterKinematic_static_64' .. OSEXT,
            'PhysXCommon_static_64' .. OSEXT,
            'PhysXExtensions_static_64' .. OSEXT,
            'PhysXFoundation_static_64' .. OSEXT,
            'PhysXPvdSDK_static_64' .. OSEXT
        }
    filter { 'architecture:x86' }
        links {
            'PhysX_static_32' .. OSEXT,
            'PhysXCharacterKinematic_static_32' .. OSEXT,
            'PhysXCommon_static_32' .. OSEXT,
            'PhysXExtensions_static_32' .. OSEXT,
            'PhysXFoundation_static_32' .. OSEXT,
            'PhysXPvdSDK_static_32' .. OSEXT
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

    filter { 'system:gaming_desktop or scarlett or windows' }
        prebuildcommands {
            '%{wks.location}/../scripts/commit.bat %{cfg.targetdir}'
        }
        prelinkcommands {
            '%{wks.location}/../scripts/copyfiles.bat %{cfg.targetdir} %{cfg.platform} %{cfg.buildcfg}'
        }
    filter { 'system:macosx' }
        prebuildcommands {
            '%{wks.location}/../scripts/commit.sh %{cfg.targetdir}'
        }
        prelinkcommands {
            '%{wks.location}/../scripts/copyfiles.sh %{cfg.targetdir} Universal %{cfg.buildcfg}'
        }
    filter { 'system:linux' }
        prebuildcommands {
            '%{wks.location}/../scripts/commit.sh %{cfg.targetdir}'
        }
        prelinkcommands {
            '%{wks.location}/../scripts/copyfiles.sh %{cfg.targetdir} %{cfg.platform} %{cfg.buildcfg}'
        }
    filter {}
