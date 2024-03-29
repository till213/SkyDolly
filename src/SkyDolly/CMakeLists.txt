qt_add_executable(${APP_NAME})

set_target_properties(${APP_NAME}
    PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/res/Info.plist.in"
)
# TODO Translations (i18n)
# qt_add_translations(${APP_NAME}
#    TS_FILES
#        i18n/${APP_NAME}_de_DE.ts
#)
target_sources(${APP_NAME}
    PRIVATE
        src/main.cpp
        src/ExceptionHandler.h src/ExceptionHandler.cpp
        src/SignalHandler.h
        src/ErrorCodes.h
        res/Info.plist.in
        res/SkyDolly.rc.in
        res/SkyDolly.exe.manifest.in
        ${QM_FILES}
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
        Sky::Widget
        Sky::UserInterface
)

if(${PLATFORM_IS_MACOS})
    target_link_libraries(${APP_NAME}
        PRIVATE
            Geoids
            Locations
    )
    include(CreateAppIconOnMacOS)
    create_app_icon_on_macos(INPUT "res/SkyDolly.png" OUTPUT APP_ICON_PATH)
    set_source_files_properties(${APP_ICON_PATH}
        PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${APP_NAME}
        PRIVATE
            src/SignalHandler_Unix.cpp
            src/UnixSignalHandler.h src/UnixSignalHandler.cpp
            ${APP_ICON_PATH}
    )
elseif(${PLATFORM_IS_WINDOWS})
    # KNOWN ISSUE: windres.exe (MinGW on Windows) does not properly handle include paths with spaces
    #              Also refer e.g. to https://bugreports.qt.io/browse/QTBUG-62918
    # WORKAROUND: Place the Sky Dolly sources into a path without spaces
    # Resource file (*.rc)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/res/SkyDolly.exe.manifest.in"
                   "${CMAKE_CURRENT_LIST_DIR}/res/SkyDolly.exe.manifest" @ONLY)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/res/SkyDolly.rc.in"
                   "${CMAKE_CURRENT_LIST_DIR}/res/SkyDolly.rc" @ONLY)
    add_library(manifest
        OBJECT
            "${CMAKE_CURRENT_LIST_DIR}/res/SkyDolly.exe.manifest"
    )
    add_library(resources
        OBJECT
            "${CMAKE_CURRENT_LIST_DIR}/res/SkyDolly.rc"
    )
    add_dependencies(resources
        manifest
    )
    if(MSVC)
        # We are embedding our own manifest via the resource (*.rc) file, so prevent MSVC linker
        # from generating and embedding a default (duplicate) manifest
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    endif()
    target_include_directories(resources PRIVATE "${CMAKE_CURRENT_LIST_DIR}/res/")
    target_sources(${APP_NAME}
        PRIVATE
            src/SignalHandler_Windows.cpp
            $<TARGET_OBJECTS:resources>
    )
else()
    target_sources(${APP_NAME}
        PRIVATE
            src/SignalHandler_Unix.cpp
            src/UnixSignalHandler.h src/UnixSignalHandler.cpp
    )
endif()
