add_definitions(-DKERNEL_EXPORT)

set(PROJECT_SOURCES
    src/KernelLib.h
    src/Aircraft.cpp
    src/Aircraft.h
    src/Position.cpp
    src/Position.h
    src/SkyMath.h
    src/AircraftInfo.cpp
    src/AircraftInfo.h
    src/VersionConfig.h.in
    src/Version.cpp
    src/Version.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Kernel SHARED
        ${PROJECT_SOURCES}
    )
else()
    add_library(Kernel SHARED
        ${PROJECT_SOURCES}
    )
endif()

configure_file(src/VersionConfig.h.in VersionConfig.h)

target_link_libraries(Kernel PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(Kernel PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Kernel PROPERTIES SOVERSION 1)

target_include_directories(Kernel PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

