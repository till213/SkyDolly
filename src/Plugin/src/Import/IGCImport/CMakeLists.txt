set(LIBRARY_NAME "IGCImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Flight Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

set(DOCUMENTATION
    doc/README.md
    doc/2022-01-31-XYY-EXAMPLE-01.IGC
    doc/2022-01-31-XYY-EXAMPLE-02.IGC
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/IGCImportPlugin.h src/IGCImportPlugin.cpp
        src/IGCParser.h src/IGCParser.cpp
        src/IGCImportSettings.h src/IGCImportSettings.cpp
        src/IGCImportOptionWidget.h src/IGCImportOptionWidget.cpp src/IGCImportOptionWidget.ui
        src/IGCImportPlugin.json
        ${DOCUMENTATION}
)

include_directories(${PLUGIN_NAME} PRIVATE ../../../../Widget/src/)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
