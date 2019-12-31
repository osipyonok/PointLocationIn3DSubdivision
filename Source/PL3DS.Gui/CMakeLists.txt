cmake_minimum_required(VERSION 3.0.0)

set(ProjectName "PL3DS.Gui")
project(${ProjectName})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB Headers "*.h")
file(GLOB Sources "*.cpp")
file(GLOB Resources "*.qrc")


add_executable(${ProjectName} ${Headers} ${Sources} ${Resources})	
set_target_properties(PL3DS.Gui PROPERTIES OUTPUT_NAME "${ProjectName}")

#Internal dependencies
target_link_libraries(${ProjectName} Math.Core)


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

					  #3rd party
					  Klein::Klein
					  )
