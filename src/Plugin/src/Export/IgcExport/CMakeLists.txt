set(LIBRARY_NAME "IGCExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)

set(DOCUMENTATION
    doc/README.md
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/IgcExportPlugin.h src/IgcExportPlugin.cpp
        src/IgcExportSettings.h src/IgcExportSettings.cpp
        src/IgcExportOptionWidget.h src/IgcExportOptionWidget.cpp src/IgcExportOptionWidget.ui
        src/IgcExportPlugin.json
        ${DOCUMENTATION}
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
