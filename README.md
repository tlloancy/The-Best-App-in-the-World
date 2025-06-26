# The Best App in the World - Cosmic Chess

Welcome to **The Best App in the World**, a cosmic chess game prototype built from scratch with C++ and powered by SDL3 and SDL3_ttf. This project is my playground to craft an epic chess experience, and it’s just getting started. Expect a stellar board, moving pieces, and more to come!

## What’s This About?
This is a work-in-progress chess game with a twist. Features so far:
- A fully rendered 8x8 chessboard with alternating colors.
- Pieces displayed as text symbols (P, N, B, R, Q, K for white; lowercase for black).
- Basic movement with mouse clicks and drag-and-drop.
- Clean window closure with 'q', Escape, or the close button.

Future goals include turn-based play, an AI opponent, move evaluation, check/checkmate detection, en passant, and sound integration!

## Getting Started

### Prerequisites
- **OS**: Works on Linux (tested on WSL Debian).
- **Dependencies**:
  - `libsdl3-dev`
  - `libsdl3-ttf-dev`
  - `cmake` (version 3.10+)
  - `g++` (version 10.2.1+)

### Installation
1. Clone the repo:

   ```bash
   git clone https://github.com/tlloancy/The-Best-App-in-the-World.git
   cd The-Best-App-in-the-World
   ```
2. Install dependencies:

   ```bash
   sudo apt update
   sudo apt install libsdl3-dev libsdl3-ttf-dev cmake g++
   ```
3. Build the project:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
4. Run it:

   ```bash
   ./CosmicChess
   ```
### Usage
- **Click to Move**: Select a piece, then click its destination.
- **Drag-and-Drop**: Click and drag a piece to its destination.
- **Controls**:
  - 'q' or Escape: Quit.
  - 'd': Toggle debug logs (on by default).
- **Close**: Use the window’s close button or 'q'.

## Development
This is an early prototype. Current limitations:
- No turn validation or collision detection yet.
- Performance could improve under heavy load.

### Contributing
Want to make it the *best*? Fork it, hack it, and submit a pull request! Ideas welcome:
- Implement turn-based logic.
- Add an AI opponent.
- Enhance graphics or add sound.

### Roadmap
- **Short-term**: Turn-based play, AI opponent, move evaluation bar.
- **Long-term**: Check/checkmate detection, en passant, castling, sound integration, cosmic visuals.

## License
[MIT License](LICENSE) - Feel free to use, modify, and share!

## Acknowledgments
- Built with love and caffeine at 4 AM.
- Thanks to SDL3 and SDL3_ttf for the power!
- Shoutout to the GitHub community for the inspiration.

---
*Made by tlloancy - Let’s make this the best app in the universe!*