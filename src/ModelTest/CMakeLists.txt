find_package(Qt5Test REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SkySearchTest "")
else()
    add_executable(SkySearchTest "")
endif()

target_sources(SkySearchTest
    PRIVATE
        src/SkySearchTest.h src/SkySearchTest.cpp
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test Model
)
if (${PLATFORM_IS_MACOS})
    list(APPEND KERNELTEST_LIBS -lc++)
endif()

target_link_libraries(SkySearchTest
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME SkySearchTest COMMAND SkySearchTest)
