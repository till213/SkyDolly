find_package(Qt6Test REQUIRED)

qt_add_executable(SkySearchTest)

target_sources(SkySearchTest
    PRIVATE
        src/SkySearchTest.h src/SkySearchTest.cpp
)

set(KERNELTEST_LIBS
    Qt6::Test
    Sky::Kernel
    Sky::Model
)

target_link_libraries(SkySearchTest
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME SkySearchTest COMMAND SkySearchTest)
