set(LIBRARY_NAME "CSVExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/CsvExportPlugin.h src/CsvExportPlugin.cpp
        src/CsvExportSettings.h src/CsvExportSettings.cpp
        src/CsvExportOptionWidget.h src/CsvExportOptionWidget.cpp src/CsvExportOptionWidget.ui
        src/CsvWriterIntf.h
        src/SkyDollyCsvWriter.h src/SkyDollyCsvWriter.cpp
        src/FlightRadar24CsvWriter.h src/FlightRadar24CsvWriter.cpp
        src/CsvExportPlugin.json
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
