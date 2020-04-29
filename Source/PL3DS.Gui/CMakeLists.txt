cmake_minimum_required(VERSION 3.0.0)

set(ProjectName "PL3DS.Gui")
project(${ProjectName})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE Headers   "include/${ProjectName}/*")
file(GLOB_RECURSE Sources   "src/*.cpp")
file(GLOB_RECURSE Forms     "src/*.ui")
file(GLOB_RECURSE Resoutces "resources/*.qrc")

add_executable(${ProjectName} ${Headers} ${Sources} ${Forms} ${Resources})    
set_target_properties(PL3DS.Gui PROPERTIES OUTPUT_NAME "${ProjectName}")

#Qt
find_package(Qt5 REQUIRED COMPONENTS 
             Core 
             Gui 
             Widgets 
             3DCore 
             3DExtras 
             3DRender 
             3DInput)

#boost
find_package(Boost REQUIRED COMPONENTS 
             system 
             filesystem)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

target_link_libraries(${ProjectName} 
                      #Qt
                      Qt5::Core 
                      Qt5::Gui 
                      Qt5::Widgets
                      Qt5::3DCore
                      Qt5::3DExtras
                      Qt5::3DRender
                      Qt5::3DInput

                      #Boost
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}

                      #Internal
                      Math.Core
                      Math.Algos
                      Math.DataStructures
                      Math.IO
                      Rendering.Core
                      Rendering.Main

                      #3rd party
                      Klein::Klein
                      )

target_include_directories(${ProjectName} PUBLIC 
                           "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           "${CMAKE_BINARY_DIR}/include")
                      

