# Directory name must match with constant values defined in the PluginManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Import)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Import)
message(STATUS CMAKE_RUNTIME_OUTPUT_DIRECTORY="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/CSVImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/IGCImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/KMLImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/GPXImport)
