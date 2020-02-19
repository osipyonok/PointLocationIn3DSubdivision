set(ProjectName "Math.Core")
project(${ProjectName})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE PublicHeaders  "include/${ProjectName}/*")
file(GLOB_RECURSE Sources 		 "src/*")

add_library(${ProjectName} SHARED ${PublicHeaders} ${Sources})

include(GenerateExportHeader)
generate_export_header(${ProjectName} 
                       EXPORT_MACRO_NAME MATH_CORE_API 
                       EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/${ProjectName}/API.h)

find_package(Qt5Core REQUIRED)

#boost
find_package(Boost REQUIRED COMPONENTS 
             system 
             )


target_link_libraries(${ProjectName} 
                      #Qt
                      Qt5::Core
                      
                      #Boost
                      ${Boost_SYSTEM_LIBRARY}
                      )

target_include_directories(${ProjectName} PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${CMAKE_BINARY_DIR}/include
                           ${Boost_INCLUDE_DIR}
                           )


#tests
include(add_unit_test_project)
add_unit_test_project(${ProjectName})
