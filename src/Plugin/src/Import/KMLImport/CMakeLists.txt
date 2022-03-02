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

set(DOCUMENTATION
    doc/README.md
    doc/FlightAware_DLH1534_LSZH_EDDF_20210817.kml
    doc/FlightRadar24_FZ981-922b3bd.kml
    doc/Generic_Track.kml
    doc/Generic_MultiTrack_Document.kml
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/KMLImportPlugin.h src/KMLImportPlugin.cpp
        src/KMLImportOptionWidget.h src/KMLImportOptionWidget.cpp src/KMLImportOptionWidget.ui
        src/KMLImportSettings.h src/KMLImportSettings.cpp
        src/KMLParserIntf.h
        src/KML.h
        src/AbstractKMLParser.h src/AbstractKMLParser.cpp
        src/AbstractKMLTrackParser.h src/AbstractKMLTrackParser.cpp
        src/FlightAwareKMLParser.h src/FlightAwareKMLParser.cpp
        src/FlightRadar24KMLParser.h src/FlightRadar24KMLParser.cpp
        src/GenericKMLParser.h src/GenericKMLParser.cpp
        src/KMLImportPlugin.json
        ${DOCUMENTATION}
)

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
