cmake_minimum_required(VERSION 3.0.0)

set(ProjectName "Math.TestingBenchmark")
project(${ProjectName})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE Sources   "src/*.cpp")

add_executable(${ProjectName} ${Sources})    
set_target_properties(${ProjectName} PROPERTIES OUTPUT_NAME "${ProjectName}")

#Qt
find_package(Qt5 REQUIRED COMPONENTS 
             Core)

#boost
find_package(Boost REQUIRED COMPONENTS 
             system)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

target_link_libraries(${ProjectName} 
                      #Qt
                      Qt5::Core 

                      #Boost
                      ${Boost_SYSTEM_LIBRARY}

                      #Internal
                      Math.Core
                      Math.Algos
                      Math.DataStructures
                      Math.IO
                      )

target_include_directories(${ProjectName} PUBLIC 
                           "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           "${CMAKE_BINARY_DIR}/include")
