set(LIBRARY_NAME "Kernel")

add_definitions(-DKERNEL_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets
)
if (${PLATFORM_IS_WINDOWS})
    find_library (PSAPI psapi)
    list(APPEND LIBRARY_DEPENDENCIES -lpsapi)
elseif (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

configure_file(src/VersionConfig.h.in VersionConfig.h)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/KernelLib.h
        src/Unit.h src/Unit.cpp
        src/Const.h
        src/Enum.h
        src/SampleRate.h
        src/SkyMath.h
        src/VersionConfig.h.in
        src/Version.h src/Version.cpp
        src/Settings.h src/Settings.cpp
        src/Replay.h
        src/Convert.h src/Convert.cpp
        src/FlightSimulator.h src/FlightSimulator.cpp
        src/Color.h src/Color.cpp
        src/File.h src/File.cpp
        src/QStringHasher.h
        src/QByteArrayHasher.h
        src/System.h src/System.cpp
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

# For generated VersionConfig.h
target_include_directories(${LIBRARY_NAME}
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
    PUBLIC
        GeographicLib::GeographicLib
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
