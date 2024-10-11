find_package(Qt6Test REQUIRED)

if(${PLATFORM_IS_MACOS})
    set_target_properties(${TEST_NAME}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/SkyDolly.app/Contents/MacOS
    )
endif()

## CSV FlightRadar24 Import Test ##
set(TEST_NAME "CsvFlightRadar24ImportTest")

qt_add_executable(${TEST_NAME})
target_sources(${TEST_NAME}
    PRIVATE
        src/AbstractFlightImportTest.h src/AbstractFlightImportTest.cpp
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
        res/PluginManagerTest.qrc
        doc/README.md
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
    Sky::Model
    Sky::PluginManager
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## CSV FlightRecorder Import Test ##
set(TEST_NAME "CsvFlightRecorderImportTest")

qt_add_executable(${TEST_NAME})
target_sources(${TEST_NAME}
    PRIVATE
        src/AbstractFlightImportTest.h src/AbstractFlightImportTest.cpp
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
        res/PluginManagerTest.qrc
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
    Sky::Model
    Sky::PluginManager
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## GPX Import Test ##
set(TEST_NAME "GpxImportTest")

qt_add_executable(${TEST_NAME})
target_sources(${TEST_NAME}
    PRIVATE
        src/AbstractFlightImportTest.h src/AbstractFlightImportTest.cpp
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
        res/PluginManagerTest.qrc
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
    Sky::Model
    Sky::PluginManager
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## KML FlightAware Import Test ##
set(TEST_NAME "KmlFlightAwareImportTest")

qt_add_executable(${TEST_NAME})
target_sources(${TEST_NAME}
    PRIVATE
        src/AbstractFlightImportTest.h src/AbstractFlightImportTest.cpp
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
        res/PluginManagerTest.qrc
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
    Sky::Model
    Sky::PluginManager
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
