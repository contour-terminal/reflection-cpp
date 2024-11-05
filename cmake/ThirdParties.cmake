set(THIRDPARTIES_HAS_FETCHCONTENT ON)
# if(${CMAKE_VERSION} VERSION_LESS 3.11)
#     set(THIRDPARTIES_HAS_FETCHCONTENT OFF)
# endif()

if(THIRDPARTIES_HAS_FETCHCONTENT)
    include(FetchContent)
    set(FETCHCONTENT_QUIET OFF)
else()
    include(DownloadProject)
endif()

if(NOT FETCHCONTENT_BASE_DIR STREQUAL "")
    set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rdparty")
endif()

set(3rdparty_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_downloads" CACHE FILEPATH "3rdparty download directory.")
message(STATUS "base dir: ${FETCHCONTENT_BASE_DIR}")
message(STATUS "dnld dir: ${3rdparty_DOWNLOAD_DIR}")

macro(ThirdPartiesAdd_Catch2)
    #find_package(Catch2 QUIET)
    find_package(Catch2)
    if (Catch2_FOUND)
        message(STATUS "Catch2 found: ${Catch2_DIR}")
    else()
        message(STATUS "Catch2 not found as system package, adding it as a thirdparty")
        set(3rdparty_Catch2_VERSION "3.4.0" CACHE STRING "Embedded catch2 version")
        set(3rdparty_Catch2_CHECKSUM "SHA256=122928b814b75717316c71af69bd2b43387643ba076a6ec16e7882bfb2dfacbb" CACHE STRING "Embedded catch2 checksum")
        set(3rdparty_Catch2_URL "https://github.com/catchorg/Catch2/archive/refs/tags/v${3rdparty_Catch2_VERSION}.tar.gz")
        set(CATCH_BUILD_EXAMPLES OFF CACHE INTERNAL "")
        set(CATCH_BUILD_EXTRA_TESTS OFF CACHE INTERNAL "")
        set(CATCH_BUILD_TESTING OFF CACHE INTERNAL "")
        set(CATCH_ENABLE_WERROR OFF CACHE INTERNAL "")
        set(CATCH_INSTALL_DOCS OFF CACHE INTERNAL "")
        set(CATCH_INSTALL_HELPERS OFF CACHE INTERNAL "")
        if(THIRDPARTIES_HAS_FETCHCONTENT)
            FetchContent_Declare(
                Catch2
                URL "${3rdparty_Catch2_URL}"
                URL_HASH "${3rdparty_Catch2_CHECKSUM}"
                DOWNLOAD_DIR "${3rdparty_DOWNLOAD_DIR}"
                DOWNLOAD_NAME "catch2-${3rdparty_Catch2_VERSION}.tar.gz"
                EXCLUDE_FROM_ALL
                DOWNLOAD_EXTRACT_TIMESTAMP 1
            )
            FetchContent_MakeAvailable(Catch2)
        else()
            download_project(
                PROJ Catch2
                URL "${3rdparty_Catch2_URL}"
                URL_HASH "${3rdparty_Catch2_CHECKSUM}"
                PREFIX "${FETCHCONTENT_BASE_DIR}/Catch2-${3rdparty_Catch2_VERSION}"
                DOWNLOAD_DIR "${3rdparty_DOWNLOAD_DIR}"
                DOWNLOAD_NAME "catch2-${3rdparty_Catch2_VERSION}.tar.gz"
                EXCLUDE_FROM_ALL
                DOWNLOAD_EXTRACT_TIMESTAMP 1
            )
        endif()
    endif()
endmacro()
