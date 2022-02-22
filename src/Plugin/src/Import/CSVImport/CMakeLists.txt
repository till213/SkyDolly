set(LIBRARY_NAME "CSVImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence Widget SkyConnect Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

set(DOCUMENTATION
    doc/FlightRadar24.csv
    doc/FlightRadar24.md
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/CSVImportPlugin.h src/CSVImportPlugin.cpp
        src/CSVImportOptionWidget.h src/CSVImportOptionWidget.cpp src/CSVImportOptionWidget.ui
        src/CSVImportSettings.h src/CSVImportSettings.cpp
        src/CSVParserIntf.h
        src/SkyDollyCSVParser.h src/SkyDollyCSVParser.cpp
        src/FlightRadar24CSVParser.h src/FlightRadar24CSVParser.cpp
        src/FlightRecorderCSVParser.h src/FlightRecorderCSVParser.cpp
        src/CSVImportPlugin.json
        ${DOCUMENTATION}
)

include_directories(${PLUGIN_NAME} PRIVATE ../../../../Widget/src/)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
