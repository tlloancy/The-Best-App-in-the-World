#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <chrono>
#include <map>
#include <thread>
#include <mutex>
#include "../core/Board.hpp"
#include "../engine/Search.hpp"

class Renderer {
public:
    Renderer(int width, int height, bool debug = false);
    ~Renderer();
    void renderBoard(Board& board, bool& isWhiteTurn);
    bool shouldClose() const;
    void handleEvents(SDL_Event& event, Board& board, bool& isWhiteTurn);
    int getSquareFromCoords(int x, int y) const;
    void setSelectedSquare(int square);
private:
    void renderChessboard();
    void loadPieceTextures();
    void renderPieces(const Board& board);
    void renderTime(const Board& board, bool isWhiteTurn);
    void renderGameEnd(const std::string& message, const SDL_Color& color, bool isQuitPrompt);
    void renderEvaluation(const SearchResult& result);
    void makeAIMove(Board& board, bool& isWhiteTurn);
    void updateSearchResult(const Board& board);
    void updateCursor(bool isDragging);
    void logDebug(const std::string& message) const;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    SDL_Cursor* cursorOpen_ = nullptr;
    SDL_Cursor* cursorClosed_ = nullptr;
    std::map<std::pair<PieceType, Color>, SDL_Texture*> textureCache_;
    std::map<std::string, SDL_Texture*> evalTextureCache_;
    std::chrono::steady_clock::time_point lastUpdate_;
    std::chrono::steady_clock::time_point lastAIMoveTime_;
    std::string basePath_;
    float whiteTime_ = 600.0f;
    float blackTime_ = 600.0f;
    bool debugEnabled_;
    bool isAIActive_ = false;
    bool isBoardFlipped_ = false;
    bool justAIMoved_ = false;
    bool quitPromptActive_ = false;
    bool quitConfirmed_ = false;
    bool isDragging_ = false;
    bool gameOver_ = false;
    int selectedSquare_ = -1;
    int dragX_ = -1;
    int dragY_ = -1;
    int windowWidth_ = 1000;
    int windowHeight_ = 800;
    Uint32 lastKeyPressTime_ = 0;
    SearchResult lastSearchResult_;
    bool searchResultValid_ = false;
    std::thread aiThread_;
    std::mutex searchMutex_;
    bool aiThreadRunning_ = false;
};

#endif
