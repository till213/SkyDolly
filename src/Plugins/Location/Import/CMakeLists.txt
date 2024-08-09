# Directory name must match with constant values defined in the PluginManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Import)
message(STATUS "Location import plugin directory=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/CsvLocationImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/KmlLocationImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SdLogLocationImport)
