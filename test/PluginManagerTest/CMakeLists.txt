find_package(Qt6Test REQUIRED)

## CSV Flight Import Test ##

set(TEST_NAME "CsvFlightImportTest")

qt_add_executable(${TEST_NAME})
if(${PLATFORM_IS_MACOS})
    set_target_properties(${TEST_NAME}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/SkyDolly.app/Contents/MacOS
    )
endif()

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
        res/PluginManagerTest.qrc
)

set(KERNELTEST_LIBS
    Qt6::Test
    Sky::Kernel
    Sky::Model
    Sky::PluginManager
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
