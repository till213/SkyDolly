set(LIBRARY_NAME "CSVImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Sky::Kernel Sky::Model Sky::Persistence Sky::Widget Sky::SkyConnect Sky::PluginManager
)

set(DOCUMENTATION
    doc/FlightRadar24.csv
    doc/FlightRadar24.md
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/CsvImportPlugin.h src/CsvImportPlugin.cpp
        src/CsvImportOptionWidget.h src/CsvImportOptionWidget.cpp src/CsvImportOptionWidget.ui
        src/CsvImportSettings.h src/CsvImportSettings.cpp
        src/CsvParserIntf.h
        src/SkyDollyCsvParser.h src/SkyDollyCsvParser.cpp
        src/FlightRadar24CsvParser.h src/FlightRadar24CsvParser.cpp
        src/FlightRecorderCsvParser.h src/FlightRecorderCsvParser.cpp
        src/CsvImportPlugin.json
        ${DOCUMENTATION}
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
