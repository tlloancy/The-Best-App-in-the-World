# The Best App in the World - Cosmic Chess

Welcome to **The Best App in the World**, a cosmic chess game prototype built from scratch with C++ and powered by SDL2 and SDL_ttf. This project is my playground to craft an epic chess experience, and it’s just getting started. Expect a stellar board, moving pieces, and more to come!

## What’s This About?
This is a work-in-progress chess game with a twist. Features so far:
- A fully rendered 8x8 chessboard with alternating colors.
- Pieces displayed as text symbols (P, N, B, R, Q, K for white; lowercase for black).
- Basic movement with mouse clicks (Knight moves in "L" shape implemented).
- Clean window closure with 'q', Escape, or the close button.

Future goals include turn-based play, advanced move validation, and maybe some cosmic flair!

## Getting Started

### Prerequisites
- **OS**: Works on Linux (tested on WSL Debian).
- **Dependencies**:
  - `libsdl2-dev`
  - `libsdl2-ttf-dev`
  - `cmake` (version 3.10+)
  - `g++` (version 10.2.1+)

### Installation
1. Clone the repo:

git clone https://github.com/tlloancy/The-Best-App-in-the-World.git
cd The-Best-App-in-the-World
text
2. Install dependencies:

sudo apt update
sudo apt install libsdl2-dev libsdl2-ttf-dev cmake g++
text
3. Build the project:

mkdir build
cd build
cmake ..
make
text
4. Run it:

./CosmicChess
text
### Usage
- **Click to Move**: Select a piece, then click its destination (Knight moves in "L").
- **Controls**:
- 'q' or Escape: Quit.
- 'd': Toggle debug logs (on by default).
- **Close**: Use the window’s close button or 'q'.

## Development
This is an early prototype. Current limitations:
- Only Knight moves are implemented.
- No turn validation or collision detection yet.
- Performance could improve under heavy load.

### Contributing
Want to make it the *best*? Fork it, hack it, and submit a pull request! Ideas welcome:
- Implement moves for all pieces.
- Add turn-based logic.
- Enhance graphics or add sound.

### Roadmap
- **Short-term**: Full piece movement, turn system.
- **Long-term**: AI opponent, cosmic visuals.

## License
[MIT License](LICENSE) - Feel free to use, modify, and share!

## Acknowledgments
- Built with love and caffeine at 4 AM.
- Thanks to SDL2 and SDL_ttf for the power!
- Shoutout to the GitHub community for the inspiration.

---
*Made by tlloancy - Let’s make this the best app in the universe!*
