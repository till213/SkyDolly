set(MODULE_NAME "CSVLocationExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/CsvLocationExportPlugin.h src/CsvLocationExportPlugin.cpp
        src/CsvLocationExportSettings.h src/CsvLocationExportSettings.cpp
        src/CsvLocationExportOptionWidget.h src/CsvLocationExportOptionWidget.cpp src/CsvLocationExportOptionWidget.ui
        src/CsvLocationWriterIntf.h
        src/CsvLocationExportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::Widget
        Sky::PluginManager
)
