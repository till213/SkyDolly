set(PLUGIN_NAME "PathCreatorPlugin")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${PLUGIN_NAME} SHARED
        ${PLUGIN_SOURCES}
    )
else()
    add_library(${PLUGIN_NAME} SHARED
        ${PLUGIN_SOURCES}
    )
endif()

target_sources(${PLUGIN_NAME}
    PRIVATE
        src/PathCreatorPlugin.cpp
        src/PathCreatorPlugin.h
        src/PathCreatorPlugin.json
)

set(PLUGIN_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model SkyConnect
)
if (${PLATFORM_IS_MACOS})
    list(APPEND PLUGIN_LIBS -lc++)
endif()

target_link_libraries(${PLUGIN_NAME} PRIVATE ${PLUGIN_LIBS})

target_include_directories(${PLUGIN_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
