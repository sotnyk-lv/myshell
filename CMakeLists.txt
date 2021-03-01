cmake_minimum_required(VERSION 3.17)
project(myshell)

set(CMAKE_CXX_STANDARD 17)

set(SERVER_SOURCES src/shell.cpp) # add source files here

find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

include_directories(inc)

add_executable(${CMAKE_PROJECT_NAME} server.cpp ${SERVER_SOURCES})

add_executable(mycat "../myshell/src/cat.cpp")

add_executable(myrls src/rls.cpp)

add_dependencies(myshell mycat myrls)

add_library(options SHARED src/options.cpp "../myshell/inc/options/options.h")
add_library(builtIn SHARED inc/builtIn/builtIn.h src/builtIn.cpp)
add_library(ioposix SHARED src/ioposix.cpp "../myshell/inc/ioposix/ioposix.h")

target_link_libraries(${CMAKE_PROJECT_NAME} builtIn ${Boost_LIBRARIES})

target_link_libraries( mycat ioposix options)