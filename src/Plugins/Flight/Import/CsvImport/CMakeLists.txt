set(MODULE_NAME "CsvImport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/CsvImportPlugin.h src/CsvImportPlugin.cpp
        src/CsvImportSettings.h src/CsvImportSettings.cpp
        src/CsvImportOptionWidget.h src/CsvImportOptionWidget.cpp src/CsvImportOptionWidget.ui
        src/CsvParserIntf.h
        src/SkyDollyCsvParser.h src/SkyDollyCsvParser.cpp
        src/FlightRadar24CsvParser.h src/FlightRadar24CsvParser.cpp
        src/FlightRecorderCsvParser.h src/FlightRecorderCsvParser.cpp
        src/CsvImportPlugin.json
        doc/FlightRadar24.csv
        doc/FlightRadar24.md
        doc/FlightRecorder.csv
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Widget
        Sky::PluginManager
)
