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
        src/Const.h
        src/Enum.h
        src/SampleRate.h
        src/SkyMath.h
        src/VersionConfig.h.in
        src/Version.cpp
        src/Version.h
        src/Settings.cpp
        src/Settings.h
)

set(KERNEL_LIBS
    Qt${QT_VERSION_MAJOR}::Core
)
if (APPLE)
    list(APPEND KERNEL_LIBS -lc++)
endif()

target_link_libraries(Kernel PRIVATE ${KERNEL_LIBS})
set_target_properties(Kernel PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Kernel PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(Kernel PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

