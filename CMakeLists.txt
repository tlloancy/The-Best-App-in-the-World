# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(CosmicChess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PkgConfig
find_package(PkgConfig REQUIRED)

# Find SDL3
find_package(SDL3 REQUIRED CONFIG)
if(NOT SDL3_FOUND)
    message(FATAL_ERROR "SDL3 not found. Please install libsdl3-dev.")
endif()

# Find SDL3_image
find_package(SDL3_image QUIET CONFIG)
if(NOT SDL3_image_FOUND)
    message(STATUS "SDL3_image not found, fetching from source...")
    include(FetchContent)
    set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(
        SDL3_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-3.2.4
    )
    FetchContent_MakeAvailable(SDL3_image)
    set(SDL3_IMAGE_INCLUDE_DIR ${sdl3_image_SOURCE_DIR}/include)
    set(SDL3_IMAGE_LIBRARY ${sdl3_image_BINARY_DIR}/libSDL3_image.so)
    set(SDL3_image_FOUND TRUE)
else()
    message(STATUS "SDL3_image found: ${SDL3_image_DIR}")
endif()

# Find SDL3_ttf
find_package(SDL3_ttf QUIET CONFIG)
if(NOT SDL3_ttf_FOUND)
    message(STATUS "SDL3_ttf not found, fetching from source...")
    include(FetchContent)
    set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(
        SDL3_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-3.2.3
    )
    FetchContent_MakeAvailable(SDL3_ttf)
    set(SDL3_TTF_INCLUDE_DIR ${sdl3_ttf_SOURCE_DIR}/include)
    set(SDL3_TTF_LIBRARY ${sdl3_ttf_BINARY_DIR}/libSDL3_ttf.so)
    set(SDL3_ttf_FOUND TRUE)
else()
    message(STATUS "SDL3_ttf found: ${SDL3_ttf_DIR}")
endif()

# Find Threads
find_package(Threads REQUIRED)

# Check for NASM and install if not found
find_program(NASM_EXECUTABLE nasm)
if(NOT NASM_EXECUTABLE)
    message(STATUS "NASM not found, attempting to install...")
    execute_process(
        COMMAND sudo apt update
        COMMAND sudo apt install -y nasm
        RESULT_VARIABLE NASM_INSTALL_RESULT
    )
    if(NASM_INSTALL_RESULT EQUAL 0)
        message(STATUS "NASM installed successfully")
        find_program(NASM_EXECUTABLE nasm REQUIRED)
    else()
        message(FATAL_ERROR "Failed to install NASM. Please install it manually with 'sudo apt install nasm'.")
    endif()
endif()

# Enable NASM
enable_language(ASM_NASM)
if(NOT CMAKE_ASM_NASM_COMPILER_LOADED)
    message(FATAL_ERROR "NASM assembler not found even after installation attempt.")
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    stockfish
    GIT_REPOSITORY https://github.com/official-stockfish/Stockfish.git
    GIT_TAG sf_17.1
)
FetchContent_MakeAvailable(stockfish)

# Include directories
include_directories(${SDL3_INCLUDE_DIRS})
include_directories(${SDL3_IMAGE_INCLUDE_DIR})
include_directories(${SDL3_TTF_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Sources
set(SOURCES
    src/main.cpp
    src/core/Board.cpp
    src/core/Evaluator.cpp
    src/core/Move.cpp
    src/core/Pawn.cpp
    src/core/Knight.cpp
    src/core/Bishop.cpp
    src/core/Rook.cpp
    src/core/Queen.cpp
    src/core/King.cpp
    src/engine/Dijkstra.cpp
    src/engine/MCTS.cpp
    src/engine/Search.cpp
    src/engine/Stockfish.cpp
    src/gui/Menu.cpp
    src/gui/Renderer.cpp
    src/gui/Spiral.cpp
    src/utils/Bitboard.cpp
    src/utils/Logger.cpp
    src/utils/Convert.cpp
    asm/popcount.asm
)

# Executable
add_executable(CosmicChess ${SOURCES})
add_custom_command(TARGET CosmicChess POST_BUILD
    COMMAND make -C ${stockfish_SOURCE_DIR}/src build ARCH=x86-64 -j
    COMMAND ${CMAKE_COMMAND} -E copy ${stockfish_SOURCE_DIR}/src/stockfish ${CMAKE_BINARY_DIR}/stockfish
    COMMAND chmod +x ${CMAKE_BINARY_DIR}/stockfish
)

# Define the path to the Stockfish binary as a preprocessor macro
target_compile_definitions(CosmicChess PRIVATE STOCKFISH_PATH="${CMAKE_BINARY_DIR}/stockfish")

# Link libraries
target_link_libraries(CosmicChess PRIVATE SDL3::SDL3)
if(SDL3_image_FOUND AND TARGET SDL3_image::SDL3_image)
    target_link_libraries(CosmicChess PRIVATE SDL3_image::SDL3_image)
else()
    target_link_libraries(CosmicChess PRIVATE ${SDL3_IMAGE_LIBRARY})
endif()
if(SDL3_ttf_FOUND AND TARGET SDL3_ttf::SDL3_ttf)
    target_link_libraries(CosmicChess PRIVATE SDL3_ttf::SDL3_ttf)
else()
    target_link_libraries(CosmicChess PRIVATE ${SDL3_TTF_LIBRARY})
endif()
target_link_libraries(CosmicChess PRIVATE Threads::Threads m)

# Set debug build
set(CMAKE_BUILD_TYPE Debug)
