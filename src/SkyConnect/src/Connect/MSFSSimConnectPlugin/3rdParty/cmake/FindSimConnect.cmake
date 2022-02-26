# SimConnect
add_library(SimConnect SHARED IMPORTED)
set_property(TARGET SimConnect PROPERTY
             IMPORTED_LOCATION "c:/MSFS SDK/SimConnect SDK/lib/SimConnect.dll")
set_property(TARGET SimConnect PROPERTY
             IMPORTED_IMPLIB "c:/MSFS SDK/SimConnect SDK/lib/SimConnect.lib")
include_directories(
    "c:/MSFS SDK/SimConnect SDK/include"
)

