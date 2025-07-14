#include "../../include/engine/Stockfish.hpp"
#include <cstring>
#include <cstdlib>
#include <unistd.h>

Stockfish::Stockfish() {
    pipe = popen("./stockfish", "r+");
    if (!pipe) {
        // Handle error
        return;
    }
    fprintf(pipe, "uci\n");
    char buf[256];
    while (fgets(buf, sizeof(buf), pipe)) {
        if (strstr(buf, "readyok")) break;
    }
}

Stockfish::~Stockfish() {
    if (pipe) {
        fprintf(pipe, "quit\n");
        pclose(pipe);
    }
}

std::pair<std::string, float> Stockfish::getBestMoveAndScore(const std::string& fen, int depth) {
    fprintf(pipe, "position fen %s\n", fen.c_str());
    fprintf(pipe, "go depth %d\n", depth);
    char buf[256];
    float score = 0.0f;
    std::string bestMove;
    while (fgets(buf, sizeof(buf), pipe)) {
        if (strstr(buf, "info")) {
            char* scoreStr = strstr(buf, "score ");
            if (scoreStr) {
                scoreStr += 6;
                if (strncmp(scoreStr, "cp ", 3) == 0) {
                    score = std::atoi(scoreStr + 3) / 100.0f;
                } else if (strncmp(scoreStr, "mate ", 5) == 0) {
                    int mateMoves = std::atoi(scoreStr + 5);
                    score = (mateMoves > 0) ? 1000.0f : -1000.0f;
                }
            }
        }
        if (strstr(buf, "bestmove")) {
            char* moveStr = strstr(buf, "bestmove ");
            if (moveStr) {
                moveStr += 9;
                char* end = strchr(moveStr, ' ');
                if (end) *end = '\0';
                bestMove = moveStr;
            }
            break;
        }
    }
    return {bestMove, score};
}
