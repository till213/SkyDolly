set(LIBRARY_NAME "IGCImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()
add_library(SD::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Flight Plugin
)

set(DOCUMENTATION
    doc/README.md
    doc/2022-01-31-XYY-EXAMPLE-01.IGC
    doc/2022-01-31-XYY-EXAMPLE-02.IGC
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/IgcImportPlugin.h src/IgcImportPlugin.cpp
        src/IgcParser.h src/IgcParser.cpp
        src/IgcImportSettings.h src/IgcImportSettings.cpp
        src/IgcImportOptionWidget.h src/IgcImportOptionWidget.cpp src/IgcImportOptionWidget.ui
        src/IgcImportPlugin.json
        ${DOCUMENTATION}
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
