set(LIBRARY_NAME "Kernel")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        KERNEL_EXPORT
)
configure_file(src/VersionConfig.h.in VersionConfig.h)
target_sources(${LIBRARY_NAME}
    PRIVATE
        src/include/Kernel/KernelLib.h
        src/include/Kernel/Color.h src/Color.cpp
        src/include/Kernel/Const.h
        src/include/Kernel/Convert.h src/Convert.cpp
        src/include/Kernel/Enum.h
        src/include/Kernel/File.h src/File.cpp
        src/include/Kernel/FlightSimulator.h src/FlightSimulator.cpp
        src/include/Kernel/QByteArrayHasher.h
        src/include/Kernel/QStringHasher.h
        src/include/Kernel/Replay.h
        src/include/Kernel/SampleRate.h
        src/include/Kernel/Settings.h src/Settings.cpp
        src/include/Kernel/SkyMath.h
        src/include/Kernel/System.h src/System.cpp
        src/include/Kernel/Unit.h src/Unit.cpp
        src/include/Kernel/Version.h src/Version.cpp
        src/VersionConfig.h.in
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include/Kernel
        # For generated VersionConfig.h
        "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        GeographicLib::GeographicLib
)

if (${PLATFORM_IS_WINDOWS})
    find_library (PSAPI psapi)
    target_link_libraries(${LIBRARY_NAME}
        PRIVATE_HEADER
            psapi
    )
    # https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-processes
    add_definitions(-DPSAPI_VERSION=1)
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/FlightSimulator_Windows.cpp
    )
elseif (${PLATFORM_IS_MACOS})
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/FlightSimulator_macOS.cpp
    )
elseif (${PLATFORM_IS_LINUX})
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/FlightSimulator_Linux.cpp
    )
endif()

set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
