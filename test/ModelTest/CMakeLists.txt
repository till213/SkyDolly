find_package(Qt6Test REQUIRED)

## SkySearch Test ##
set(TEST_NAME "SkySearchTest")

qt_add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
    Sky::Model
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
