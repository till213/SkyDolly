qt_add_executable(${APP_NAME})
set_target_properties(${APP_NAME}
    PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/res/Info.plist.in"
)
# TODO Translations (i18n)
#qt_add_translations(${APP_NAME}
#    TS_FILES
#        i18n/${APP_NAME}_de_DE.ts
#)
target_sources(${APP_NAME}
    PRIVATE
        src/main.cpp
        src/ExceptionHandler.h src/ExceptionHandler.cpp
        res/Info.plist.in
        res/SkyDolly.rc.in
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
            ${APP_ICON_PATH}
    )
elseif(${PLATFORM_IS_WINDOWS})
    # KNOWN ISSUE: windres.exe (MinGW on Windows) does not properly handle include paths with spaces
    #              Also refer e.g. to https://bugreports.qt.io/browse/QTBUG-62918
    # WORKAROUND: Place the Sky Dolly sources into a path without spaces
    # Resource file (*.rc)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/res/SkyDolly.rc.in"
                   "${CMAKE_CURRENT_SOURCE_DIR}/res/SkyDolly.rc" @ONLY)
    add_library(resources OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/res/SkyDolly.rc")
    target_include_directories(resources PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/res/")
    target_sources(${APP_NAME}
        PRIVATE
            $<TARGET_OBJECTS:resources>
    )
endif()
