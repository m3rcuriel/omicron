#pragma once
#include <vector>
#include <tuple>
#include <map>

#include "chess.h"

namespace chess {

namespace agent {

constexpr size_t kNumParticles = 1000;

class StateDistribution {
public:
    StateDistribution(std::vector<Board>&& boards) : particles(std::move(boards)) {}
    StateDistribution(StateDistribution&&) = default;
    StateDistribution(const StateDistribution&) = default;

    Board sample() const;

    // Update the board and return the fraction of games won by that move.
    double update(Move move);

    double heuristic_value(Color color);

    // Move each particle randomly, splitting into equivalence classes by captured piece
    // Returns [(weight, capture, distribution)]
    std::vector<std::tuple<int, Capture, StateDistribution>>
        update_random(Color opponent_color) const;

    std::vector<Move> get_available_actions(Color color) const;

private:
    std::vector<Board> particles;
};

}

}
