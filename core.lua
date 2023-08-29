project 'Core'

    default_project_settings()

    kind 'SharedLib'

    files {
        'core/**.h',
        'core/**.cpp'
    }

    defines {
        'CORE'
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

    filter { 'system:gaming_desktop or scarlett or windows or macosx or linux' }
        files {
            'deps-public/src/**'
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
