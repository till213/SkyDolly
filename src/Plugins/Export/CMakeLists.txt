# Directory name must match with constant values defined in the PluginManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Export)
message(STATUS "Output export plugin directory=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/CsvExport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/GpxExport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/IgcExport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/JsonExport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/KmlExport)
