set(MODULE_NAME "CSVImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/CsvImportPlugin.h src/CsvImportPlugin.cpp
        src/CsvImportOptionWidget.h src/CsvImportOptionWidget.cpp src/CsvImportOptionWidget.ui
        src/CsvImportSettings.h src/CsvImportSettings.cpp
        src/CsvParserIntf.h
        src/SkyDollyCsvParser.h src/SkyDollyCsvParser.cpp
        src/FlightRadar24CsvParser.h src/FlightRadar24CsvParser.cpp
        src/FlightRecorderCsvParser.h src/FlightRecorderCsvParser.cpp
        src/CsvImportPlugin.json
        doc/FlightRadar24.csv
        doc/FlightRadar24.md
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Widget
        Sky::PluginManager
)
