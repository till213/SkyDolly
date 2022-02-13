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
        src/KMLExportDialog.h src/KMLExportDialog.cpp src/KMLExportDialog.ui
        src/KMLStyleExport.h src/KMLStyleExport.cpp
        src/KMLExportPlugin.json
)

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
