set(LIBRARY_NAME "KMLImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Sky::Kernel Sky::Model Sky::Flight Sky::Persistence Sky::PluginManager Sky::Widget
)

set(DOCUMENTATION
    doc/README.md
    doc/FlightAware_DLH1534_LSZH_EDDF_20210817.kml
    doc/FlightRadar24_FZ981-922b3bd.kml
    doc/Generic_Track.kml
    doc/Generic_MultiTrack_Document.kml
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/KmlImportPlugin.h src/KmlImportPlugin.cpp
        src/KmlImportOptionWidget.h src/KmlImportOptionWidget.cpp src/KmlImportOptionWidget.ui
        src/KmlImportSettings.h src/KmlImportSettings.cpp
        src/KmlParserIntf.h
        src/Kml.h
        src/AbstractKmlParser.h src/AbstractKmlParser.cpp
        src/AbstractKmlTrackParser.h src/AbstractKmlTrackParser.cpp
        src/FlightAwareKmlParser.h src/FlightAwareKmlParser.cpp
        src/FlightRadar24KmlParser.h src/FlightRadar24KmlParser.cpp
        src/GenericKmlParser.h src/GenericKmlParser.cpp
        src/KmlImportPlugin.json
        ${DOCUMENTATION}
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
