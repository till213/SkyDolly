set(CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})
if(NOT DEFINED PRE_CONFIGURE_DIR)
    set(PRE_CONFIGURE_DIR ${CURRENT_LIST_DIR})
endif()
message(STATUS "GitInfo: PRE_CONFIGURE_DIR: ${PRE_CONFIGURE_DIR}")

if(NOT DEFINED POST_CONFIGURE_DIR)
    set(POST_CONFIGURE_DIR ${CMAKE_BINARY_DIR}/generated)
endif()

set(PRE_CONFIGURE_FILE "${PRE_CONFIGURE_DIR}/GitInfo.cpp.in")
set(POST_CONFIGURE_FILE "${POST_CONFIGURE_DIR}/GitInfo.cpp")

function(CheckGitWrite git_hash)
    file(WRITE ${CMAKE_BINARY_DIR}/git-state.txt ${git_hash})
endfunction()

function(CheckGitRead git_hash)
    if(EXISTS ${CMAKE_BINARY_DIR}/git-state.txt)
        file(STRINGS ${CMAKE_BINARY_DIR}/git-state.txt CONTENT)
        LIST(GET CONTENT 0 var)

        set(${git_hash} ${var} PARENT_SCOPE)
    endif()
endfunction()

function(CheckGitVersion)
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    CheckGitRead(GIT_HASH_CACHE)
    if(NOT EXISTS ${POST_CONFIGURE_DIR})
        file(MAKE_DIRECTORY ${POST_CONFIGURE_DIR})
    endif()

    if(NOT EXISTS ${POST_CONFIGURE_DIR}/GitInfo.h)
        file(COPY ${PRE_CONFIGURE_DIR}/GitInfo.h DESTINATION ${POST_CONFIGURE_DIR})
    endif()

    if(NOT DEFINED GIT_HASH_CACHE)
        set(GIT_HASH_CACHE "INVALID")
    endif()

    # Only update the GitInfo.cpp if the hash has changed. This will
    # prevent us from rebuilding the project more than we need to.
    if(NOT ${GIT_HASH} STREQUAL "${GIT_HASH_CACHE}" OR NOT EXISTS ${POST_CONFIGURE_FILE})
        # Set che GIT_HASH_CACHE variable the next build won't have
        # to regenerate the source file.
        CheckGitWrite(${GIT_HASH})
        message(STATUS "GitInfo: Git hash is different: ${GIT_HASH} - rebuilding GitInfo")

        # Also get the date of the latest commit hash of the working branch
        execute_process(
            COMMAND git show --no-patch --no-notes --pretty=%cd --date=iso-strict ${GIT_HASH}
            WORKING_DIRECTORY ${CURRENT_LIST_DIR}
            OUTPUT_VARIABLE GIT_ISO_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        configure_file(${PRE_CONFIGURE_FILE} ${POST_CONFIGURE_FILE} @ONLY)
    else()
        message(STATUS "GitInfo: Git hash is equal: ${GIT_HASH} - no rebuild of GitInfo")
    endif()

endfunction()

function(CheckGitSetup)

    add_custom_target(AlwaysCheckGit COMMAND ${CMAKE_COMMAND}
        -DRUN_CHECK_GIT_VERSION=1
        -DPRE_CONFIGURE_DIR=${PRE_CONFIGURE_DIR}
        -DPOST_CONFIGURE_FILE=${POST_CONFIGURE_DIR}
        -DGIT_HASH_CACHE=${GIT_HASH_CACHE}
        -P ${CURRENT_LIST_DIR}/CheckGit.cmake
        BYPRODUCTS ${POST_CONFIGURE_FILE}
        )

    add_library(GitInfo STATIC)
    target_sources(GitInfo
        PRIVATE
            ${POST_CONFIGURE_FILE}
    )
    target_include_directories(GitInfo PUBLIC ${CMAKE_BINARY_DIR}/generated)
    add_dependencies(GitInfo AlwaysCheckGit)

    CheckGitVersion()
endfunction()

# This is used to run this function from an external cmake process.
if(RUN_CHECK_GIT_VERSION)
    CheckGitVersion()
endif()
