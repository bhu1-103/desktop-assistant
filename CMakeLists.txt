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

# Prevent whisper.cpp from building its server, tests, and examples (no repo bloat!)
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_SERVER OFF CACHE BOOL "" FORCE)
# Build whisper.cpp from the git submodule
add_subdirectory(external/whisper.cpp)

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
    whisper
    ${SDL2_LIBRARIES}
)
