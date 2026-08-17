cmake_minimum_required(VERSION 3.16)

project(friday
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# LOWWI being used a library to compile the code in this repo rather than the one in the LOWWI repo itself.
set(CLFML_LOWWI_BUILD_EXAMPLE_PROJECTS OFF CACHE BOOL "" FORCE)

# Build LOWWI from the git submodule, no more repo bloat...
add_subdirectory(external/lowwi)

# SDL2 for audio_async.
find_package(SDL2 REQUIRED)

# Executableeeeeeeeee!!
add_executable(friday
    src/main.cpp
    src/audio_async.cpp
)

target_include_directories(friday PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(friday PRIVATE
    CLFML::lowwi
    ${SDL2_LIBRARIES}
)
