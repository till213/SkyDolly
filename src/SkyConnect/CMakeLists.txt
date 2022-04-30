set(LIBRARY_NAME "SkyConnect")

add_definitions(-DSKYCONNECT_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(SD::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Core
    SD::Kernel
    SD::Model
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/SkyConnectLib.h
        src/SkyConnectManager.cpp
        src/SkyConnectManager.h
        src/SkyConnectIntf.h
        src/Connect.h
        src/AbstractSkyConnect.cpp
        src/AbstractSkyConnect.h
)

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
