#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "core/Board.hpp"
#include "../utils/Logger.hpp"

struct Matrix4 {
    float data[16];
    Matrix4() { for (int i = 0; i < 16; ++i) data[i] = (i % 5 == 0) ? 1.0f : 0.0f; }
    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result; for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) {
            result.data[i + j * 4] = 0; for (int k = 0; k < 4; ++k)
                result.data[i + j * 4] += data[i + k * 4] * other.data[k + j * 4];
        } return result;
    }
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    void renderBoard(const Board& board);
    void renderSpiral(float evaluation);
    bool shouldClose();
    void swapBuffers();
    bool isKeyPressed(int key) const;
    void closeWindow();
    void handleClick(int x, int y, Board& board);
private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    int selectedSquare_ = -1;
    bool debugEnabled_ = false;
    void renderPiece(PieceType type, Color color, int x, int y);
    void logDebug(const std::string& message);
};