include 'premake5-base.lua'

workspace 'Tools'
    default_workspace_settings()

    filter { 'system:windows' }
        platforms { 'x86', 'x64', 'ARM64' }
    filter {}

project 'vpktool'
    default_project_settings()

    files {
        'include/fs.h',
        'include/game.h',
        'include/vpk2.h',
        'src/fs.cpp',
        'src/vpk2.cpp',
        'tools/tool.h',
        'tools/toolbase.cpp',
        'tools/vpktool.cpp'
    }

