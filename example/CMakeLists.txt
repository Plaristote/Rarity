cmake_minimum_required(VERSION 3.0)

project(rarity-example)

find_package(PkgConfig)
pkg_check_modules(RARITY REQUIRED librarity)

file(GLOB_RECURSE sourceFiles
     main.cpp
     rarity-bindings.cpp)

include_directories(${RARITY_INCLUDE_DIRS})
link_directories(${RARITY_LIBRARY_DIRS})
add_executable(rarity-example ${sourceFiles})
target_link_libraries(rarity-example ${RARITY_LIBRARIES})
