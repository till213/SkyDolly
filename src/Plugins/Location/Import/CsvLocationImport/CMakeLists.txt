set(MODULE_NAME "CsvLocationImport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/CsvLocationImportPlugin.h src/CsvLocationImportPlugin.cpp
        src/CsvLocationImportSettings.h src/CsvLocationImportSettings.cpp
        src/CsvLocationImportOptionWidget.h src/CsvLocationImportOptionWidget.cpp src/CsvLocationImportOptionWidget.ui
        src/CsvLocationParserIntf.h
        src/SkyDollyCsvLocationParser.h src/SkyDollyCsvLocationParser.cpp
        src/LittleNavmapCsvParser.h src/LittleNavmapCsvParser.cpp
        src/CsvLocationImportPlugin.json
        doc/Example.csv
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::Widget
        Sky::PluginManager
)
