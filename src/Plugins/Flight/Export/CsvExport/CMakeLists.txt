set(MODULE_NAME "CSVExport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/CsvExportPlugin.h src/CsvExportPlugin.cpp
        src/CsvExportSettings.h src/CsvExportSettings.cpp
        src/CsvExportOptionWidget.h src/CsvExportOptionWidget.cpp src/CsvExportOptionWidget.ui
        src/CsvWriterIntf.h
        src/SkyDollyCsvWriter.h src/SkyDollyCsvWriter.cpp
        src/FlightRadar24CsvWriter.h src/FlightRadar24CsvWriter.cpp
        src/PositionAndAttitudeCsvWriter.h src/PositionAndAttitudeCsvWriter.cpp
        src/CsvExportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
