set(SKYMATH_SOURCES
    src/SkyMathTest.cpp
    src/SkyMathTest.h
)

set(SKYSEARCH_SOURCES
    src/SkySearchTest.cpp
    src/SkySearchTest.h
)

find_package(Qt5Test REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SkyMathTest
        ${SKYMATH_SOURCES}
    )
    qt_add_executable(SkySearchTest
        ${SKYSEARCH_SOURCES}
    )
else()
    add_executable(SkyMathTest
        ${SKYMATH_SOURCES}
    )
    add_executable(SkySearchTest
        ${SKYSEARCH_SOURCES}
    )
endif()

target_link_libraries(SkyMathTest PRIVATE Qt${QT_VERSION_MAJOR}::Test Kernel)
target_link_libraries(SkySearchTest PRIVATE Qt${QT_VERSION_MAJOR}::Test Kernel)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_link_libraries(SkyMathTest PRIVATE Qt${QT_VERSION_MAJOR}::Test Kernel -lc++)
    target_link_libraries(SkySearchTest PRIVATE Qt${QT_VERSION_MAJOR}::Test Kernel -lc++)
else()
    target_link_libraries(SkyMathTest PRIVATE Qt${QT_VERSION_MAJOR}::Test Kernel)
    target_link_libraries(SkySearchTest PRIVATE Qt${QT_VERSION_MAJOR}::Test Kernel)
endif()

add_test(NAME SkyMathTest COMMAND SkyMathTest)
add_test(NAME SkySearchTest COMMAND SkySearchTest)


