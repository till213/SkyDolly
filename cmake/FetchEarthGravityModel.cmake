include(FetchContent)

set(CMAKE_TLS_VERIFY true)

option(SD_FETCH_EGM "Fetch earth gravity model EGM2008-5"  OFF)
if(SD_FETCH_EGM)
    FetchContent_Declare(
        egm
        URL https://downloads.sourceforge.net/project/geographiclib/geoids-distrib/egm2008-5.tar.bz2?use_mirror=autoselect
        URL_HASH SHA256=9A57C14330AC609132D324906822A9DA9DE265AD9B9087779793EB7080852970
        DOWNLOAD_NO_EXTRACT false
    )
    FetchContent_MakeAvailable(egm)
endif()
