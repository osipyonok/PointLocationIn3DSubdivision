set(ProjectName "Math.Core")
project(${ProjectName})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

file(GLOB PublicHeaders  "include/${ProjectName}/*.h")
file(GLOB PrivateHeaders "*.h")
file(GLOB Sources 		 "*.cpp")

add_library(${ProjectName} SHARED ${PublicHeaders} ${PrivateHeaders} ${Sources})

include(GenerateExportHeader)
generate_export_header(${ProjectName} 
                       EXPORT_MACRO_NAME MATH_CORE_API 
                       EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/${ProjectName}/API.h)

find_package(Qt5Core REQUIRED)

target_link_libraries(${ProjectName} 
                      #Qt
                      Qt5::Core)

target_include_directories(${ProjectName} PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${CMAKE_BINARY_DIR}/include)
