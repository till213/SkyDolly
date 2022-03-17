set(LIBRARY_NAME "KMLExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/KMLExportPlugin.h src/KMLExportPlugin.cpp
        src/KMLExportSettings.h src/KMLExportSettings.cpp
        src/KMLExportOptionWidget.h src/KMLExportOptionWidget.cpp src/KMLExportOptionWidget.ui
        src/KMLStyleExport.h src/KMLStyleExport.cpp
        src/KMLExportPlugin.json
)

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
