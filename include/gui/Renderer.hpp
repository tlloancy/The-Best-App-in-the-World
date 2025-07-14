#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "../core/Board.hpp"
#include "../core/Move.hpp"
#include "../engine/Search.hpp"
#include "../engine/MCTS.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <map>

class Renderer {
public:
    Renderer(int width, int height, bool debug);
    ~Renderer();
    bool shouldClose() const;
    void renderBoard(Board& board, bool& isWhiteTurn);
    void handleEvents(SDL_Event& event, Board& board, bool& isWhiteTurn);
private:
    void updateSearchResult(const Board& board, bool isWhiteTurn);
    void makeAIMove(Board& board, bool& isWhiteTurn);
    void renderChessboard();
    void loadPieceTextures();
    void renderPieces(const Board& board);
    void renderTime(const Board& board, bool isWhiteTurn);
    void renderGameEnd(const std::string& message, const SDL_Color& color, bool isQuitPrompt);
    void renderEvaluation(const SearchResult& result);
    void updateCursor(bool isDragging);
    int getSquareFromCoords(int x, int y) const;
    void setSelectedSquare(int square);
    void logDebug(const std::string& message) const;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    SDL_Cursor* cursorOpen_ = nullptr;
    SDL_Cursor* cursorClosed_ = nullptr;
    std::string basePath_;
    std::map<std::pair<PieceType, Color>, SDL_Texture*> textureCache_;
    std::map<std::string, SDL_Texture*> evalTextureCache_;
    std::vector<SDL_FPoint> stars_;
    bool debugEnabled_;
    int windowWidth_;
    int windowHeight_;
    bool quitPromptActive_ = false;
    bool quitConfirmed_ = false;
    bool isDragging_ = false;
    int selectedSquare_ = -1;
    int dragX_ = -1;
    int dragY_ = -1;
    bool gameOver_ = false;
    bool isAIActive_ = false;
    bool useStockfish_ = false;
    bool isBoardFlipped_ = false;
    bool aiThreadRunning_ = false;
    bool justAIMoved_ = false;
    std::thread aiThread_;
    std::mutex searchMutex_;
    SearchResult lastSearchResult_;
    bool searchResultValid_ = false;
    float whiteTime_ = 600.0f;
    float blackTime_ = 600.0f;
    std::chrono::steady_clock::time_point lastUpdate_;
    std::chrono::steady_clock::time_point lastAIMoveTime_;
};

#endif
