#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "../../include/core/Board.hpp"
#include "../../include/engine/Search.hpp"
#include "../../include/engine/MCTS.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class Renderer {
public:
    Renderer(int width, int height, bool debug);
    ~Renderer();

    bool shouldClose() const;
    void renderBoard(Board& board, bool& isWhiteTurn);
    void handleEvents(SDL_Event& event, Board& board, bool& isWhiteTurn);
    void updateSearchResult(const Board& board, bool isWhiteTurn);

private:
    bool debugEnabled_;
    int windowWidth_, windowHeight_;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    TTF_Font* boldFont_ = nullptr;
    SDL_Cursor* cursorOpen_ = nullptr;
    SDL_Cursor* cursorClosed_ = nullptr;
    std::string basePath_;
    std::map<std::pair<PieceType, Color>, SDL_Texture*> textureCache_;
    std::map<std::string, SDL_Texture*> evalTextureCache_;
    std::vector<SDL_FPoint> stars_;
    int selectedSquare_ = -1;
    bool isDragging_ = false;
    int dragX_ = -1, dragY_ = -1;
    float whiteTime_ = 600.0f, blackTime_ = 600.0f;
    std::chrono::steady_clock::time_point lastUpdate_;
    std::chrono::steady_clock::time_point lastAIMoveTime_;
    bool gameOver_ = false;
    bool quitPromptActive_ = false;
    bool quitConfirmed_ = false;
    bool isAIActive_ = false;
    bool isBoardFlipped_ = false;
    bool useStockfish_ = false;
    bool showEvaluation_ = true;
    bool justAIMoved_ = false;
    SearchResult lastSearchResult_;
    std::vector<SearchResult> topSearchResults_;
    bool searchResultValid_ = false;
    std::mutex searchMutex_;
    std::thread aiThread_;
    bool aiThreadRunning_ = false;
    std::vector<Board> moveHistory_;
    int historyIndex_ = -1;
    std::pair<int, int> premove_ = {-1, -1};
    MCTSSearch* mctsSearch_ = nullptr;
    std::atomic<bool> aiInitialized_ = false;
    std::thread evalThread_;
    bool evalThreadRunning_ = false;

    void makeAIMove(Board& board, bool& isWhiteTurn);
    int getSquareFromCoords(int x, int y) const;
    void setSelectedSquare(int square);
    void logDebug(const std::string& message) const;
    void updateCursor(bool isDragging);
    void renderChessboard();
    void loadPieceTextures();
    void renderPieces(const Board& board);
    void renderTime(const Board& board, bool isWhiteTurn);
    void renderGameEnd(const std::string& message, const SDL_Color& color, bool isQuitPrompt);
    void renderEvaluation(const SearchResult& result, const std::vector<SearchResult>& topResults);
    void renderNavigationButtons();
    void renderLoadingScreen(float progress);
    bool isPointInButton(int x, int y, int buttonX, int buttonY, int buttonW, int buttonH) const;
};

#endif // RENDERER_HPP
