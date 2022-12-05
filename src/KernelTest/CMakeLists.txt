find_package(Qt5Test REQUIRED)

## SkyMath Test ##

set(TEST_NAME "SkyMathTest")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TEST_NAME})
else()
    add_executable(${TEST_NAME})
endif()

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## Sort Test ##

set(TEST_NAME "SortTest")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TEST_NAME})
else()
    add_executable(${TEST_NAME})
endif()

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## Name Test ##

set(TEST_NAME "NameTest")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TEST_NAME})
else()
    add_executable(${TEST_NAME})
endif()

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## PositionParser test ##

set(TEST_NAME "PositionParserTest")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TEST_NAME})
else()
    add_executable(${TEST_NAME})
endif()

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

## CsvParser test ##

set(TEST_NAME "CsvParserTest")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TEST_NAME})
else()
    add_executable(${TEST_NAME})
endif()

target_sources(${TEST_NAME}
    PRIVATE
        src/${TEST_NAME}.h src/${TEST_NAME}.cpp
)

set(KERNELTEST_LIBS
    Qt${QT_VERSION_MAJOR}::Test
    Sky::Kernel
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${KERNELTEST_LIBS}
)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
