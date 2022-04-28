find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

set(GIT_SUBMODULE_TEST_FILES
    "${PROJECT_SOURCE_DIR}/3rdParty/geographiclib/CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/3rdParty/ordered-map/CMakeLists.txt"
)
cmake_print_variables(SUBMODULE_TEST_FILES)
foreach(TEST_FILE IN LISTS SUBMODULE_TEST_FILES)
    if(NOT EXISTS ${TEST_FILE})
        message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
    endif()
endforeach()
