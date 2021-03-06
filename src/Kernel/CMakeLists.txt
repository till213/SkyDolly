add_definitions(-DKERNEL_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Kernel SHARED "")
else()
    add_library(Kernel SHARED "")
endif()

configure_file(src/VersionConfig.h.in VersionConfig.h)

target_sources(Kernel
    PRIVATE
        src/KernelLib.h
        src/Unit.cpp
        src/Unit.h
        src/Const.h
        src/Enum.h
        src/SampleRate.h
        src/SkyMath.h
        src/VersionConfig.h.in
        src/Version.cpp
        src/Version.h
        src/Settings.cpp
        src/Settings.h
        src/Replay.h
        src/Convert.h
        src/FlightSimulator.cpp
        src/FlightSimulator.h
)

if (${PLATFORM_IS_WINDOWS})
    # https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-processes
    add_definitions(-DPSAPI_VERSION=1)
    target_sources(Kernel
        PRIVATE
            src/FlightSimulator_Windows.cpp
    )
elseif (${PLATFORM_IS_MACOS})
    target_sources(Kernel
        PRIVATE
            src/FlightSimulator_macOS.cpp
    )
elseif (${PLATFORM_IS_LINUX})
    target_sources(Kernel
        PRIVATE
            src/FlightSimulator_Linux.cpp
    )
endif()

set(KERNEL_LIBS
    Qt${QT_VERSION_MAJOR}::Core
)
if (${PLATFORM_IS_WINDOWS})
    find_library (PSAPI psapi)
    list(APPEND KERNEL_LIBS -lpsapi)
elseif (${PLATFORM_IS_MACOS})
    list(APPEND KERNEL_LIBS -lc++)
endif()

target_link_libraries(Kernel PRIVATE ${KERNEL_LIBS})
set_target_properties(Kernel PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Kernel PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(Kernel PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

