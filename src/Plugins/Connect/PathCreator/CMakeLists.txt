set(LIBRARY_NAME "PathCreator")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets
    Sky::Kernel
    Sky::Model
    Sky::PluginManager
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/PathCreatorPlugin.cpp
        src/PathCreatorPlugin.h
        src/PathCreatorPlugin.json
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
