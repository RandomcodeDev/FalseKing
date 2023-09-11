project 'Core'

    default_project_settings()

    location 'build/%{prj.name}.%{wks.name}'
    filter { 'system:not macosx' }
        targetname '%{prj.name}.%{wks.name}.%{cfg.platform}'
    filter { 'system:macosx' }
        targetname '%{prj.name}.%{wks.name}.Universal'
    filter {}

    filter { 'system:psp' }
        kind 'StaticLib'
    filter { 'system:not psp' }
        kind 'SharedLib'
    filter {}

    if isTools ~= nil then
        files {
            'core/core.h',
            'core/fs.*',
            'core/globals.cpp',
            'core/stdafx.*',
            'core/vpk.*',
            'core/Core.rc'
        }

        defines {
            'TOOL'
        }
    else
        files {
            'core/**.h',
            'core/**.cpp',
            'core/Core.rc'
        }
    end

    defines {
        'CORE'
    }
    
    prebuildcommands {
        'python3 %{wks.location}/../scripts/commit.py %{cfg.targetdir}'
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
    
    filter { 'system:gaming_desktop or scarlett or windows or macosx or linux or psp' }
        files {
            'deps-public/src/*.h',
            'deps-public/src/*.c',
            'deps-public/src/*.hpp',
            'deps-public/src/*.cpp',
            'deps-public/src/*.cc'
        }
        if isTools == nil then
            files {
                'deps-public/src/**'
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

            filter { 'system:not macosx', 'system:not psp', 'architecture:not x86' }
                links {
                    'PhysX_static_64',
                    'PhysXCharacterKinematic_static_64',
                    'PhysXCommon_static_64',
                    'PhysXExtensions_static_64',
                    'PhysXFoundation_static_64',
                    'PhysXPvdSDK_static_64'
                }
            filter { 'system:not macosx', 'system:not psp', 'architecture:x86' }
                links {
                    'PhysX_static_32',
                    'PhysXCharacterKinematic_static_32',
                    'PhysXCommon_static_32',
                    'PhysXExtensions_static_32',
                    'PhysXFoundation_static_32',
                    'PhysXPvdSDK_static_32'
                }
            filter { 'system:psp' }
            	links {
                    'PhysX_static',
                    'PhysXCharacterKinematic_static',
                    'PhysXCommon_static',
                    'PhysXExtensions_static',
                    'PhysXFoundation_static',
                    'PhysXPvdSDK_static'
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
        end
    filter {}
