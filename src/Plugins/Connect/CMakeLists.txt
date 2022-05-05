# Directory name must match with constant values defined in the SkyConnectManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Connect)
message(STATUS "Output connect plugin directory=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/PathCreator)
if (${PLATFORM_IS_WINDOWS})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/MSFSSimConnectPlugin)
endif()
