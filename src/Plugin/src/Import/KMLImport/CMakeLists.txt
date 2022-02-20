set(LIBRARY_NAME "KMLImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Flight Persistence Plugin Widget
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/KMLImportPlugin.h src/KMLImportPlugin.cpp
        src/KMLImportOptionWidget.h src/KMLImportOptionWidget.cpp src/KMLImportOptionWidget.ui
        src/KMLImportSettings.h src/KMLImportSettings.cpp
        src/KMLParserIntf.h
        src/KML.h
        src/FlightAwareKMLParser.h src/FlightAwareKMLParser.cpp
        src/FlightRadar24KMLParser.h src/FlightRadar24KMLParser.cpp
        src/GenericKMLParser.h src/GenericKMLParser.cpp
        src/KMLImportPlugin.json
)

include_directories(${PLUGIN_NAME} PRIVATE ../../../../Widget/src/)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
