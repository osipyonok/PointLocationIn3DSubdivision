set(ProjectName "Rendering.Main")
project(${ProjectName})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE PublicHeaders  "include/${ProjectName}/*")
file(GLOB_RECURSE Sources 		 "src/*")

add_library(${ProjectName} SHARED ${PublicHeaders} ${Sources})

include(GenerateExportHeader)
generate_export_header(${ProjectName} 
					   EXPORT_MACRO_NAME RENDERING_MAIN_API 
					   EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/${ProjectName}/API.h)


find_package(Qt5 REQUIRED COMPONENTS Core)

target_link_libraries(${ProjectName} 
					  #Qt
					  Qt5::Core
					  
					  #Internal
					  Rendering.Core
					  
					  #3rd party
					  Klein::Klein)


target_include_directories(${ProjectName} PUBLIC 
						   "${CMAKE_CURRENT_SOURCE_DIR}/include"
						   "${CMAKE_BINARY_DIR}/include")
