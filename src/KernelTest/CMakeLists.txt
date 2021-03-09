find_package(Qt5Test REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SkyMathTest "")
    qt_add_executable(SkySearchTest "")
else()
    add_executable(SkyMathTest "")
    add_executable(SkySearchTest "")
endif()

target_sources(SkyMathTest
    PRIVATE
    src/SkyMathTest.cpp
    src/SkyMathTest.h
)
target_sources(SkySearchTest
    PRIVATE
    src/SkySearchTest.cpp
    src/SkySearchTest.h
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test Kernel
)
if (APPLE)
    list(APPEND KERNELTEST_LIBS -lc++)
endif()

target_link_libraries(SkyMathTest PRIVATE ${KERNELTEST_LIBS})
target_link_libraries(SkySearchTest PRIVATE ${KERNELTEST_LIBS})

add_test(NAME SkyMathTest COMMAND SkyMathTest)
add_test(NAME SkySearchTest COMMAND SkySearchTest)
