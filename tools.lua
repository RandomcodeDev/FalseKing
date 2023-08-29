workspace 'Tools'

    -- Override the specified target and build for the host
    system(os.host())

    default_workspace_settings()

    filter { 'system:windows' }
        platforms { 'x86', 'x64', 'ARM64' }
        toolset 'msc'
    filter {}

include 'core.lua'

project 'vpktool'

    default_project_settings()

    files {
        'tools/tool.h',
        'tools/toolbase.cpp',
        'tools/vpktool.cpp'
    }

    links {
        'Core'
    }
