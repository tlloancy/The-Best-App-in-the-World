#ifndef CONVERT_HPP
#define CONVERT_HPP

#include "../core/Move.hpp"
#include <string>

namespace Convert {
    uint8_t squareFromAlg(const std::string& alg);
    Move fromUCI(const std::string& uci);
}

#endif
