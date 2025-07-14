#include "../../include/utils/Convert.hpp"

namespace Convert {
    uint8_t squareFromAlg(const std::string& alg) {
        if (alg.length() < 2) return 64;
        int file = alg[0] - 'a';
        int rank = alg[1] - '1';
        if (file < 0 || file > 7 || rank < 0 || rank > 7) return 64;
        return rank * 8 + file;
    }

    Move fromUCI(const std::string& uci) {
        if (uci.length() < 4) return {64, 64};
        uint8_t from = squareFromAlg(uci.substr(0, 2));
        uint8_t to = squareFromAlg(uci.substr(2, 2));
        return {from, to};
    }
}
