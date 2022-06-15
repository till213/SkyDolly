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
configure_file(src/VersionConfig.h.in generated/VersionConfig.h @ONLY)

target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Kernel/KernelLib.h
        include/Kernel/Color.h src/Color.cpp
        include/Kernel/Const.h
        include/Kernel/Convert.h src/Convert.cpp
        include/Kernel/Enum.h
        include/Kernel/File.h src/File.cpp
        include/Kernel/FlightSimulator.h src/FlightSimulator.cpp
        include/Kernel/Replay.h
        include/Kernel/SampleRate.h
        include/Kernel/Settings.h src/Settings.cpp
        include/Kernel/SkyMath.h
        include/Kernel/System.h src/System.cpp
        include/Kernel/Unit.h src/Unit.cpp
        include/Kernel/Version.h src/Version.cpp
        include/Kernel/QUuidHasher.h
        include/Kernel/Sort.h
        src/VersionConfig.h.in
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        include/Kernel
        # For generated files: VersionConfig.h, GitInfo.h
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        GeographicLib::GeographicLib
        tsl::ordered_map
    PRIVATE
        GitInfo
)

if(${PLATFORM_IS_WINDOWS})
    find_library (PSAPI psapi)
    target_link_libraries(${LIBRARY_NAME}
        PRIVATE
            psapi
    )
    # https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-processes
    target_compile_definitions(${LIBRARY_NAME}
        PRIVATE
            PSAPI_VERSION=1
    )
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/FlightSimulator_Windows.cpp
    )
elseif(${PLATFORM_IS_MACOS})
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/FlightSimulator_macOS.cpp
    )
elseif(${PLATFORM_IS_LINUX})
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
