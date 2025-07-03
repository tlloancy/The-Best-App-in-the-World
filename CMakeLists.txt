# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(CosmicChess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajoute SDL3 et SDL3_ttf avec pkg-config
find_package(PkgConfig REQUIRED)
find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS} ${SDL3_TTF_INCLUDE_DIRS})

# Utilise des chemins relatifs basés sur CMAKE_SOURCE_DIR
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Sources
set(SOURCES
    src/main.cpp
    src/core/Board.cpp
    src/core/Evaluator.cpp
    src/core/Move.cpp
    src/core/Piece.cpp
    src/core/Pawn.cpp
    src/core/Knight.cpp
    src/core/Bishop.cpp
    src/core/Rook.cpp
    src/core/Queen.cpp
    src/core/King.cpp
    src/engine/Dijkstra.cpp
    src/engine/MCTS.cpp
    src/engine/Search.cpp
    src/gui/Renderer.cpp
    src/gui/Spiral.cpp
    src/utils/Bitboard.cpp
    src/utils/Logger.cpp
    asm/popcount.asm
)

# Cible executable
add_executable(CosmicChess ${SOURCES})

# Link SDL3 et SDL3_ttf
target_link_libraries(CosmicChess SDL3::SDL3 SDL3_ttf::SDL3_ttf m)

# Option debug
set(CMAKE_BUILD_TYPE Debug)