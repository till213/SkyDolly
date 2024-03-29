set(LIBRARY_NAME "Flight")

qt_add_library(${LIBRARY_NAME} SHARED)
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        FLIGHT_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Flight/FlightLib.h
        include/Flight/FlightAugmentation.h src/FlightAugmentation.cpp
        include/Flight/Analytics.h src/Analytics.cpp
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
        include/Flight
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt6::Core
    PRIVATE
        Sky::Kernel
        Sky::Model
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
