cmake_minimum_required(VERSION 3.13.0)

project(PointLocationIn3DSubdivision)

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo" CACHE STRING 
    "Semicolon separated list of supported configuration types, only supports ${CMAKE_CONFIGURATION_TYPES} anything else will be ignored." FORCE)
set(BUILD_OUTPUT_DIR "${CMAKE_BINARY_DIR}/x64" CACHE STRING "Where to put your *.exe, *dll, etc." FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


file(GLOB PatchFolders "${CMAKE_SOURCE_DIR}/Patches/*")
if(PatchFolders)
    include(FindGit)
    if(GIT_FOUND)
        message(STATUS "-- Found Git: ${GIT_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Can't find Git")
    endif()
    
    foreach(PatchFolder ${PatchFolders})
        file(GLOB Patches "${PatchFolder}/*.patch")
        get_filename_component(ThirdPartyFolder ${PatchFolder} NAME)
        set(PathToBePatched "ThirdParty/${ThirdPartyFolder}")
        execute_process(
                        COMMAND "${GIT_EXECUTABLE}" "submodule" "update" "--progress" "--init" "--" "ThirdParty/${ThirdPartyFolder}"
                        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                        OUTPUT_QUIET
                        ERROR_QUIET)
        foreach(Patch ${Patches})
            message(STATUS "Applying patch ${Patch}")
            #abort previous failed patch application (if any)
            execute_process(
                            COMMAND "${GIT_EXECUTABLE}" "am" "--abort"
                            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${PathToBePatched}"
                            OUTPUT_QUIET
                            ERROR_QUIET)
            execute_process(
                            COMMAND "${GIT_EXECUTABLE}" "am" "--3way" "--ignore-space-change" "--keep-cr" "${Patch}"
                            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${PathToBePatched}"
                            OUTPUT_VARIABLE Output
                            RESULT_VARIABLE Result)
            if(Result EQUAL 0)
                message(STATUS "Patch applied: ${Patch}")
            else()
                message(FATAL_ERROR "Error applying patch: ${Patch}, ${Output}")
            endif()
        endforeach(Patch)
    endforeach(PatchFolder)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeUtilities")

add_subdirectory(Source)
add_subdirectory(ThirdParty)
