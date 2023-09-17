require 'premake-consoles/consoles'

function default_workspace_settings()
    location 'build'
    filename '%{wks.name}-%{_ACTION or \'\'}-%{_TARGET_OS}-%{os.target() or \'\'}'

    configurations { 'Debug', 'Release', 'Retail' }

    filter { 'system:windows' }
        platforms { 'x86', 'ARM64' }
    filter { 'system:gaming_desktop' }
        platforms { 'Gaming.Desktop.x64' }
    filter { 'system:scarlett' }
        platforms { 'Gaming.Xbox.Scarlett.x64' }
    filter { 'system:linux' }
        platforms { 'x86_64', 'ARM64' }
    filter { 'system:psp' }
    	platforms { 'PSP' }
    filter {}
        
    filter { 'system:windows', 'platforms:x86' }
        toolset 'msc-v141_xp'
    filter {}

    filter { 'platforms:ARM64' }
        architecture 'arm64'
    filter {}

    files {
        '.github/**',
        'depscripts/*',
        'scripts/*',
        '.clang-format',
        '.gitattributes',
        '.gitignore',
        '.gitmodules',
        'DESIGN.md',
        'LICENSE.txt',
        'logo.png',
        '*.lua',
        'README.md',
    }
end

function default_project_settings()
    location 'build/%{prj.name}'
    filename '%{prj.name}-%{_ACTION or \'\'}-%{_TARGET_OS or \'\'}-%{os.target() or \'\'}'

    language 'C++'
    cppdialect 'C++17'

    filter { 'system:not macosx' }
        targetname '%{prj.name}.%{cfg.platform}'
        objdir '%{prj.location}/%{cfg.platform}/%{cfg.buildcfg}'
        targetdir '%{wks.location}/%{cfg.platform}/%{cfg.buildcfg}'
    filter { 'system:macosx' }
        targetname '%{prj.name}.Universal'
        targetextension ''
        objdir '%{prj.location}/Universal/%{cfg.buildcfg}'
        targetdir '%{wks.location}/Universal/%{cfg.buildcfg}'
    filter {}

    characterset 'MBCS'

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
        runtime 'Debug'
    filter { 'configurations:Release' }
        defines 'NDEBUG'
        optimize 'On'
        symbols 'On'
        runtime 'Release'
    filter { 'configurations:Retail' }
        defines { 'NDEBUG', 'RETAIL' }
        optimize 'On'
        runtime 'Release'
    filter {}

    includedirs {
        _MAIN_SCRIPT_DIR,
        '%{cfg.targetdir}'
    }

    kind 'ConsoleApp'

    filter { 'toolset:msc' }
        disablewarnings {
            4307,
            4251
        }
    filter {}

    filter { 'system:gaming_desktop or scarlett or windows or macosx or linux or psp' }
        includedirs {
            'deps-public/include',
        }

        frameworkdirs {
            'deps-public/Frameworks/%{cfg.buildcfg}',
            'deps-public/Frameworks'
        }
    filter { 'system:macosx' }
        libdirs {
            'deps-public/lib/Universal/%{cfg.buildcfg}',
            'deps-public/lib/Universal'
        }
    filter { 'system:gaming_desktop or scarlett or windows or linux or psp' }
        libdirs {
            'deps-public/lib/%{cfg.architecture}/%{cfg.buildcfg}',
            'deps-public/lib/%{cfg.architecture}',
            'deps-public/lib/%{cfg.architecture}/Release' -- This is for Retail builds
        }
    filter {}
end

include 'tools.lua'
include 'game.lua'
