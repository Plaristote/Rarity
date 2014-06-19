cmake_minimum_required(VERSION 2.8)

find_package(Ruby REQUIRED)

include_directories(../include ${RUBY_INCLUDE_DIRS})
link_directories(${RUBY_LIBDIRS})

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "-std=c++0x")
elseif(APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++11")
endif()

file(GLOB_RECURSE sourceFiles
     main.cpp
     rarity-bindings.cpp)

add_executable(rarity-example ${sourceFiles})

target_link_libraries(rarity-example ruby)
