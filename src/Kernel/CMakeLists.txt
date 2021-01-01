add_definitions(-DKERNEL_EXPORT)

set(PROJECT_SOURCES
    src/KernelLib.h
    src/SkyDolly.cpp
    src/SkyDolly.h
    src/Aircraft.cpp
    src/Aircraft.h
    src/Position.cpp
    src/Position.h
    src/SkyMath.h
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

target_link_libraries(Kernel PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(Kernel PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Kernel PROPERTIES SOVERSION 1)
