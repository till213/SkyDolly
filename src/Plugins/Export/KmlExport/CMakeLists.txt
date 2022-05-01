set(LIBRARY_NAME "KMLExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Sky::Kernel Sky::Model Sky::PluginManager
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/KmlExportPlugin.h src/KmlExportPlugin.cpp
        src/KmlExportSettings.h src/KmlExportSettings.cpp
        src/KmlExportOptionWidget.h src/KmlExportOptionWidget.cpp src/KmlExportOptionWidget.ui
        src/KmlStyleExport.h src/KmlStyleExport.cpp
        src/KmlExportPlugin.json
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
