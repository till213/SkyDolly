find_package(Qt6Test REQUIRED)

## SkyMath Test ##
set(TEST_NAME "SkyMathTest")

qt_add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## Sort Test ##
set(TEST_NAME "SortTest")

qt_add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## Name Test ##
set(TEST_NAME "NameTest")

qt_add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## PositionParser test ##
set(TEST_NAME "PositionParserTest")

qt_add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## CsvParser test ##
set(TEST_NAME "CsvParserTest")

qt_add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(TEST_LIBS
    Qt6::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${TEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
