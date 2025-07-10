#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <chrono>
#include <map>
#include "../core/Board.hpp"

class Renderer;
extern Renderer* globalRenderer;

class Renderer {
public:
    Renderer();
    ~Renderer();
    void renderBoard(const Board& board, bool isWhiteTurn);
    void renderPiece(PieceType type, Color color, int x, int y);
    void renderGameEnd(const std::string& message, const SDL_Color& color, bool isQuitPrompt = false);
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
    int getSelectedSquare() const { return selectedSquare_; }
    void setSelectedSquare(int square) { selectedSquare_ = square; }
    bool getDebugEnabled() const { return debugEnabled_; }

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    SDL_Cursor* cursorOpen_ = nullptr;
    SDL_Cursor* cursorClosed_ = nullptr;
    int selectedSquare_ = -1;
    int dragX_ = -1;
    int dragY_ = -1;
    std::chrono::steady_clock::time_point lastUpdate_;
    std::chrono::steady_clock::time_point lastMoveTime_;
    float whiteTime_ = 600.0f;
    float blackTime_ = 600.0f;
    bool debugEnabled_ = true;
    bool lastStateChanged = false;
    bool isDragging_ = false;
    std::string basePath_;
    std::map<std::pair<PieceType, Color>, SDL_Texture*> textureCache_;
    bool quitPromptActive_ = false;
    bool quitConfirmed_ = false;
    Uint32 lastKeyPressTime_ = 0;
};

#endif
