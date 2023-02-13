# Directory name must match with constant values defined in the PluginManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Import)
message(STATUS "Flight import plugin directory=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

# TODO RE-ENABLE ME!!!
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/CsvImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/GpxImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/IgcImport)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/KmlImport)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SdLogImport)
