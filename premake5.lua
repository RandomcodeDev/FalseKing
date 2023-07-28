workspace 'Game'

    location 'build'

    configurations { 'Debug', 'Release' }

    filter { 'system:windows' }
        platforms { 'Gaming.Desktop.x64', 'Gaming.Scarlett.x64', 'x86', 'ARM64' }
    filter {}

    filter { 'system:linux' }
        platforms { 'x64', 'ARM64' }
    filter {}
        
    filter { 'toolset:msc' }
        toolset 'v141_xp'
    filter {}

    filter { 'platforms:Gaming.Desktop.x64' }
        architecture 'x86_64'
    filter { 'platforms:Gaming.Scarlett.x64' }
        architecture 'x86_64'
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
    
    filter { 'configurations:Debug' }
        kind 'ConsoleApp'
    filter { 'configurations:Release' }
        kind 'WindowedApp'
    filter {}

    files {
        'premake5.lua',
        'include/**.h',
        'src/**.cpp',
    }

    removefiles {
        'src/sdl.cpp',
    }

    includedirs {
        'include'
    }

    filter { 'system:windows or macosx or linux' }
        includedirs {
            'deps-public/include',
        }

        files {
            'deps-public/src/**',
            'src/sdl.cpp'
        }
    filter {}

    filter { 'system:windows' }
        defines {
            'NOMINMAX',
            '_CRT_SECURE_NO_DEPRECATE',
            '_CRT_SECURE_NO_WARNINGS',
        }
    filter {}

    flags { 'MultiProcessorCompile' }
    staticruntime 'Off'
    
    filter { 'configurations:Debug' }
        defines '_DEBUG'
        symbols 'On'
    filter { 'configurations:Release' }
        defines 'NDEBUG'
        optimize 'On'
    filter {}
