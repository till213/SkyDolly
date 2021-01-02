set(PROJECT_SOURCES
    src/SkyMathTest.cpp
    src/SkyMathTest.h
)

find_package(Qt5Test REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SkyMathTest
        ${PROJECT_SOURCES}
    )
else()
    add_executable(SkyMathTest
        ${PROJECT_SOURCES}
    )
endif()

enable_testing(true)
add_test(NAME SkyMathTest COMMAND SkyMathTest)

target_link_libraries(SkyMathTest PRIVATE Qt${QT_VERSION_MAJOR}::Test Kernel)

