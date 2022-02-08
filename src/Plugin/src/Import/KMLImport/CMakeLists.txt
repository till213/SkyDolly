set(LIBRARY_NAME "KMLImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Flight Persistence Widget SkyConnect Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/KMLImportPlugin.h src/KMLImportPlugin.cpp
        src/KMLImportDialog.h src/KMLImportDialog.cpp src/KMLImportDialog.ui
        src/KMLImportSettings.h src/KMLImportSettings.cpp
        src/KMLImportPlugin.json
)

include_directories(${PLUGIN_NAME} PRIVATE ../../../../Widget/src/)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
