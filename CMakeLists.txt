# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(CosmicChess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajoute SDL2 et SDL_ttf avec pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

# Sources (sans InputHandler temporairement)
set(SOURCES
    src/main.cpp
    src/core/Board.cpp
    src/core/Evaluator.cpp
    src/core/Move.cpp
    src/core/Piece.cpp
    src/engine/Dijkstra.cpp
    src/engine/MCTS.cpp
    src/engine/Search.cpp
    src/gui/Renderer.cpp
    src/gui/Spiral.cpp
    src/utils/Bitboard.cpp
    src/utils/Logger.cpp
    asm/popcount.asm
)

# Inclure les headers
include_directories(include)

# Cible executable
add_executable(CosmicChess ${SOURCES})

# Link SDL2 et SDL_ttf
target_link_libraries(CosmicChess ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

# Option debug
set(CMAKE_BUILD_TYPE Debug)
