# Directory name must match with constant values defined in the SkyConnectManager.cpp
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Connect)
message(STATUS "Output connect plugin directory=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/PathCreator)
if(${PLATFORM_IS_WINDOWS})
    find_package(SimConnect REQUIRED)
    if(${MSFS_SDK_INSTALLED})
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/MSFSSimConnectPlugin)
    else()
        message(STATUS "MSFS SDK not installed, skipping MSFSSimConnectPlugin ${MSFS_SDK_FOUND}")
    endif()
endif()
