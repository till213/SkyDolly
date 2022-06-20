# Directory name must match with constant values defined in the ModuleManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Module)
message(STATUS "Output module plugin directory=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Logbook)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Formation)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Location)
