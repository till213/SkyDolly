find_package(Qt5Test REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SkyMathTest "")
else()
    add_executable(SkyMathTest "")
endif()

target_sources(SkyMathTest
    PRIVATE
        src/SkyMathTest.cpp
        src/SkyMathTest.h
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test Kernel
)
if (${PLATFORM_IS_MACOS})
    list(APPEND KERNELTEST_LIBS -lc++)
endif()

target_link_libraries(SkyMathTest PRIVATE ${KERNELTEST_LIBS})

add_test(NAME SkyMathTest COMMAND SkyMathTest)
