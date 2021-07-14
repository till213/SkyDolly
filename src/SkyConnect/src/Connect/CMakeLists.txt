# Directory name must match with constant values defined in the SkyConnectManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/connect)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/connect)
message(STATUS CMAKE_RUNTIME_OUTPUT_DIRECTORY="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/PathCreator)
if (WIN32)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/FS2020SimConnectPlugin)
endif()
