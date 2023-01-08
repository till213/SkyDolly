set(MODULE_NAME "KMLImport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
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
        doc/README.md
        doc/FlightAware_DLH1534_LSZH_EDDF_20210817.kml
        doc/FlightRadar24_FZ981-922b3bd.kml
        doc/Generic_Track.kml
        doc/Generic_MultiTrack_Document.kml
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Widget
        Sky::PluginManager
)
