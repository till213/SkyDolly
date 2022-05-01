set(LIBRARY_NAME "Flight")

add_definitions(-DFLIGHT_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Core
    Sky::Kernel
    Sky::Model
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/FlightLib.h
        src/FlightAugmentation.h src/FlightAugmentation.cpp
        src/Analytics.h src/Analytics.cpp
)

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
