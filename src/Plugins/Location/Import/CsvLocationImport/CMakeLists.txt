set(MODULE_NAME "CSVLocationImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/CsvLocationImportPlugin.h src/CsvLocationImportPlugin.cpp
        src/CsvLocationImportSettings.h src/CsvLocationImportSettings.cpp
        src/CsvLocationImportOptionWidget.h src/CsvLocationImportOptionWidget.cpp src/CsvLocationImportOptionWidget.ui
        src/CsvLocationImportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Widget
        Sky::PluginManager
)
