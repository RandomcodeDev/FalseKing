workspace 'Tools'

    system(os.host())

    default_workspace_settings()

    filter { 'system:windows' }
        platforms { 'x86', 'x64', 'ARM64' }
        toolset 'msc'
    filter {}

project 'vpktool'

    default_project_settings()

    files {
        'core/fs.cpp',
        'core/fs.h',
        'core/game.h',
        'core/vpk2.cpp',
        'core/vpk2.h',
        'core/stdafx.h',
        'tools/tool.h',
        'tools/toolbase.cpp',
        'tools/vpktool.cpp'
    }
