set(LIBRARY_NAME "UserInterface")

# Used to populate the Info.plist.in and SkyDolly.rc.in
set(PROJECT_APPLICATION_NAME "Sky Dolly")
set(PROJECT_ORGANISATION_NAME till213)

qt_add_executable(${APP_NAME})
set_target_properties(${APP_NAME}
    PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist.in"
)

target_sources(${APP_NAME}
    PRIVATE
        src/main.cpp
        res/Info.plist.in
        res/SkyDolly.rc.in
)

target_include_directories(${APP_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated
)
target_link_libraries(${APP_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::PluginManager
        Sky::UserInterface
)
