# SimConnect
# Checks the environment variable MSFS_SDK for the SDK location.
# Defines the variable MSFS_SDK_INSTALLED in case the MSFS SDK is installed, based on the existence of the MSFS_SDK directory
add_library(SimConnect SHARED IMPORTED)
add_library(MSFS::SimConnect ALIAS SimConnect)

if(DEFINED ENV{MSFS_SDK})
    set(MSFS_SDK_PATH $ENV{MSFS_SDK} )
    message(STATUS "MSFS_SDK environment variable set: ${MSFS_SDK_PATH}")
else()
    set(MSFS_SDK_PATH "c:/MSFS SDK/")
    message(STATUS "MSFS_SDK environment variable not set, fallback to: ${MSFS_SDK_PATH}")
endif()

if(EXISTS "${MSFS_SDK_PATH}")
    set_property(TARGET SimConnect PROPERTY IMPORTED_LOCATION "${MSFS_SDK_PATH}/SimConnect SDK/lib/SimConnect.dll")
    set_property(TARGET SimConnect PROPERTY IMPORTED_IMPLIB "${MSFS_SDK_PATH}/SimConnect SDK/lib/SimConnect.lib")
    target_include_directories(SimConnect
        INTERFACE
            "${MSFS_SDK_PATH}/SimConnect SDK/include"
    )
    set(MSFS_SDK_INSTALLED true)
    message(STATUS "MSFS SDK installed at ${MSFS_SDK_PATH}")
else()
    message(WARNING "MSFS SDK is not installed")
endif()
