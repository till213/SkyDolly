# SimConnect
add_library(SimConnect SHARED IMPORTED)
add_library(MSFS::SimConnect ALIAS SimConnect)

if(DEFINED ENV{MSFS_SDK})
    set(SDK_PREFIX $ENV{MSFS_SDK} )
    message(STATUS "MSFS_SDK environment variable set: $ENV{MSFS_SDK}")
else()
    set(SDK_PREFIX "c:/MSFS SDK/")
    message(STATUS "MSFS_SDK environment variable not set, fallback to: ${SDK_PREFIX}")
endif()

set_property(TARGET SimConnect PROPERTY IMPORTED_LOCATION "${SDK_PREFIX}/SimConnect SDK/lib/SimConnect.dll")
set_property(TARGET SimConnect PROPERTY IMPORTED_IMPLIB "${SDK_PREFIX}/SimConnect SDK/lib/SimConnect.lib")
target_include_directories(SimConnect
    INTERFACE
        "${SDK_PREFIX}/SimConnect SDK/include"
)
