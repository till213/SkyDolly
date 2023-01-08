set(MODULE_NAME "CSVLocationExport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/CsvLocationExportPlugin.h src/CsvLocationExportPlugin.cpp
        src/CsvLocationExportSettings.h src/CsvLocationExportSettings.cpp
        src/CsvLocationExportOptionWidget.h src/CsvLocationExportOptionWidget.cpp src/CsvLocationExportOptionWidget.ui
        src/CsvLocationWriterIntf.h
        src/SkyDollyCsvLocationWriter.h src/SkyDollyCsvLocationWriter.cpp
        src/LittleNavmapCsvLocationWriter.h src/LittleNavmapCsvLocationWriter.cpp
        src/CsvLocationExportPlugin.json
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
