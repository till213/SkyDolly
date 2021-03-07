add_definitions(-DSKYCONNECT_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(SkyConnect SHARED "")
else()
    add_library(SkyConnect SHARED "")
endif()

target_sources(SkyConnect
    PRIVATE
        src/SkyConnectLib.h
        src/SkyManager.cpp
        src/SkyManager.h
        src/SkyConnectIntf.h
        src/Connect.h
)

if (WIN32)
    # SimConnect
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdParty/cmake")
    find_package(SimConnect REQUIRED)

    target_sources(SkyConnect
        PRIVATE
            src/SkyConnectImpl.cpp
            src/SkyConnectImpl.h
            src/SimConnectDataDefinition.h
            src/SimConnectAircraftData.cpp
            src/SimConnectAircraftData.h
            src/SimConnectAircraftInfo.cpp
            src/SimConnectAircraftInfo.h
        )

     # Copy SimConnect.dll to bin directory (for development)
    add_custom_command(TARGET SkyConnect POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SimConnect>
            $<TARGET_FILE_DIR:SkyConnect>)
    set(SKYCONNNECT_LIBS SimConnect)
else()
    target_sources(SkyConnect
        PRIVATE
            src/SkyConnectDummy.cpp
            src/SkyConnectDummy.h
        )

    set(SKYCONNNECT_LIBS)

endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    list(APPEND SKYCONNNECT_LIBS -lc++)
endif()

target_link_libraries(SkyConnect PRIVATE Qt${QT_VERSION_MAJOR}::Core ${SKYCONNNECT_LIBS} Kernel)

set_target_properties(SkyConnect PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(SkyConnect PROPERTIES SOVERSION 1)



