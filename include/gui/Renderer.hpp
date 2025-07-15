#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "../../include/core/Board.hpp"
#include "../../include/gui/Menu.hpp"
#include "../../include/engine/Search.hpp"
#include "../../include/engine/MCTS.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <map>

enum class GameState {
    Menu,
    Board
};

struct Star {
    float x;
    float y;
};

class Renderer {
public:
    Renderer(int width, int height, bool debug, bool enableMenu);
    ~Renderer();
    bool shouldClose() const;
    void updateSearchResult(const Board& board, bool isWhiteTurn);
    void renderBoard(Board& board, bool& isWhiteTurn);
    int getSquareFromCoords(int x, int y) const;
    void setSelectedSquare(int square);
    void logDebug(const std::string& message) const;
    void updateCursor(bool isDragging);
    void handleEvents(SDL_Event& event, Board& board, bool& isWhiteTurn);
private:
    void renderLoadingScreen(float progress);
    void renderChessboard();
    void loadPieceTextures();
    void renderPieces(const Board& board);
    void renderTime(const Board& board, bool isWhiteTurn);
    void renderGameEnd(const std::string& message, const SDL_Color& color, bool isQuitPrompt);
    void renderEvaluation(const SearchResult& result);
    void renderNavigationButtons();
    bool isPointInButton(int x, int y, int buttonX, int buttonY, int buttonW, int buttonH) const;
    void makeAIMove(Board& board, bool& isWhiteTurn);

    bool debugEnabled_;
    bool enableMenu_;
    int windowWidth_;
    int windowHeight_;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    TTF_Font* boldFont_ = nullptr;
    TTF_Font* smallFont_ = nullptr;
    std::string basePath_;
    std::vector<Board> moveHistory_;
    int historyIndex_ = 0;
    std::vector<Star> stars_;
    bool isDragging_ = false;
    int selectedSquare_ = -1;
    int dragX_ = 0;
    int dragY_ = 0;
    SDL_Cursor* cursorOpen_ = nullptr;
    SDL_Cursor* cursorClosed_ = nullptr;
    float whiteTime_ = 600.0f;
    float blackTime_ = 600.0f;
    std::chrono::steady_clock::time_point lastUpdate_;
    std::chrono::steady_clock::time_point lastAIMoveTime_;
    bool gameOver_ = false;
    bool quitPromptActive_ = false;
    bool quitConfirmed_ = false;
    Menu* menu_ = nullptr;
    GameState gameState_ = GameState::Board;
    MCTSSearch* mctsSearch_ = nullptr;
    bool aiThreadRunning_ = false;
    std::thread aiThread_;
    std::mutex searchMutex_;
    SearchResult lastSearchResult_;
    bool searchResultValid_ = false;
    bool isAIActive_ = false;
    bool isBoardFlipped_ = false;
    bool showEvaluation_ = true;
    bool aiInitialized_ = false;
    bool justAIMoved_ = false;
    bool useStockfish_ = false;
    std::pair<int, int> premove_ = {-1, -1};
    std::map<std::pair<PieceType, Color>, SDL_Texture*> textureCache_;
    std::map<std::string, SDL_Texture*> evalTextureCache_;
};

#endif // RENDERER_HPP
