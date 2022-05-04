# SimConnect
add_library(SimConnect SHARED IMPORTED)
add_library(MSFS::SimConnect ALIAS SimConnect)
set_property(TARGET SimConnect
    PROPERTY
        IMPORTED_LOCATION "c:/MSFS SDK/SimConnect SDK/lib/SimConnect.dll"
)
set_property(TARGET SimConnect
    PROPERTY
        IMPORTED_IMPLIB "c:/MSFS SDK/SimConnect SDK/lib/SimConnect.lib"
)
target_include_directories(SimConnect
    "c:/MSFS SDK/SimConnect SDK/include"
)

