# On macOS place the tests into the same bundle directory like the app itself,
# specifically for testing the plugins
if(${PLATFORM_IS_MACOS})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${APP_NAME}.app/Contents/MacOS)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/KernelTest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ModelTest)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/PluginManagerTest)
