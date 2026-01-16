cmake_minimum_required(VERSION 3.10)
project(strategy_nodes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    include
    sgg
    sgg/sgg
)

add_executable(strategy_nodes
    src/main.cpp
    src/GlobalState.cpp
    src/Node.cpp
    src/Unit.cpp
)

target_link_directories(strategy_nodes PRIVATE sgg/lib)

target_link_libraries(strategy_nodes
    sgg
    SDL2
    SDL2_mixer
    GLEW
    freetype
    GL
    GLU
)
