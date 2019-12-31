set(ProjectName "Math.Core")
project(${ProjectName})

add_definitions(-DMATH_CORE_LIB)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

file(GLOB PublicHeaders  "include/${ProjectName}/*.h")
file(GLOB PrivateHeaders "*.h")
file(GLOB Sources 		 "*.cpp")

add_library(${ProjectName} SHARED ${PublicHeaders} ${PrivateHeaders} ${Sources})

#Qt
find_package(Qt5Core REQUIRED)
target_link_libraries(${ProjectName} Qt5::Core)


target_include_directories(${ProjectName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
