set(LIBRARY_NAME "IGCImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence Flight Widget SkyConnect Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/IGCImportPlugin.h src/IGCImportPlugin.cpp
        src/IGCImportPlugin.json
)

include_directories(${PLUGIN_NAME} PRIVATE ../../../../Widget/src/)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
