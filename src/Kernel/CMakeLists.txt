set(LIBRARY_NAME "Kernel")

qt_add_library(${LIBRARY_NAME} SHARED)
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        KERNEL_EXPORT
)
configure_file(src/VersionConfig.h.in generated/VersionConfig.h @ONLY)

# https://jonathanhamberg.com/post/cmake-embedding-git-hash/
set(PRE_CONFIGURE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
include(${PROJECT_SOURCE_DIR}/cmake/CheckGit.cmake)
CheckGitSetup()

target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Kernel/KernelLib.h
        include/Kernel/Color.h src/Color.cpp
        include/Kernel/Const.h
        include/Kernel/Convert.h src/Convert.cpp
        include/Kernel/CsvParser.h src/CsvParser.cpp
        include/Kernel/Enum.h
        include/Kernel/File.h src/File.cpp
        include/Kernel/FlightSimulator.h
        include/Kernel/Name.h
        include/Kernel/PositionParser.h src/PositionParser.cpp
        include/Kernel/RecentFile.h src/RecentFile.cpp
        include/Kernel/Replay.h
        include/Kernel/SampleRate.h
        include/Kernel/SecurityToken.h
        include/Kernel/Settings.h src/Settings.cpp
        include/Kernel/SkyMath.h
        include/Kernel/System.h src/System.cpp
        include/Kernel/Unit.h src/Unit.cpp
        include/Kernel/Version.h src/Version.cpp
        include/Kernel/QUuidHasher.h
        include/Kernel/QStringHasher.h
        include/Kernel/Sort.h
        include/Kernel/StackTrace.h src/StackTrace.cpp
        src/SettingsConverterV0dot13.h
        src/SettingsConverterV0dot16.h
        src/SettingsConverterV0dot17.h
        src/SettingsConverterV0dot17dot5.h
        src/SettingsConverter.h src/SettingsConverter.cpp
        src/VersionConfig.h.in
        src/GitInfo.h src/GitInfo.cpp.in
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
        include/Kernel
        # For generated files: VersionConfig.h, GitInfo.h
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt6::Widgets
        GeographicLib::GeographicLib
        tsl::ordered_map
    PRIVATE
        GitInfo
        cpptrace::cpptrace
)

# Platform-specific implementations
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
            src/SecurityToken_Default.cpp
    )
elseif(${PLATFORM_IS_MACOS})
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/FlightSimulator_macOS.cpp
            src/SecurityToken_macOS.mm
    )
elseif(${PLATFORM_IS_LINUX})
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/FlightSimulator_Linux.cpp
            src/SecurityToken_Default.cpp
    )
endif()

set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
