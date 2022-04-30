set(LIBRARY_NAME "GPXExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()
add_library(SD::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/GpxExportPlugin.h src/GpxExportPlugin.cpp
        src/GpxExportSettings.h src/GpxExportSettings.cpp
        src/GpxExportOptionWidget.h src/GpxExportOptionWidget.cpp src/GpxExportOptionWidget.ui
        src/GpxExportPlugin.json
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
