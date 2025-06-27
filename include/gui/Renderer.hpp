#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <chrono>
#include "../core/Board.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void renderBoard(const Board& board, bool isWhiteTurn);
    void renderPiece(PieceType type, Color color, int x, int y);
    void renderSpiral(float evaluation);
    void renderEvaluation(float eval);
    void renderTurnIndicator(bool isWhiteTurn);
    void swapBuffers();
    bool shouldClose();
    void handleEvents(const SDL_Event& event, Board& board, bool& isWhiteTurn);
    int getSquareFromCoords(int x, int y);
    void updateCursor(bool isDragging);
    void logDebug(const std::string& message);
    bool isKeyPressed(int key) const;
    void closeWindow();
    int getSelectedSquare() const { return selectedSquare_; } // Getter
    void setSelectedSquare(int square) { selectedSquare_ = square; } // Setter

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    SDL_Cursor* cursorOpen_ = nullptr;
    SDL_Cursor* cursorClosed_ = nullptr;
    std::chrono::steady_clock::time_point lastUpdate_;
    std::chrono::steady_clock::time_point lastMoveTime_;
    int whiteTime_ = 600; // 10 minutes en secondes
    int blackTime_ = 600;
    int selectedSquare_ = -1;
    bool debugEnabled_ = false;
};
