#pragma once
#include <vector>
#include <tuple>
#include <map>

#include "chess.h"

namespace chess {

namespace agent {

constexpr size_t kNumParticlesRollout = 100;
constexpr size_t kNumParticles = 1000000;

struct Observation {
    std::array<std::array<Piece, 3>, 3> obs;

    // Top-left
    Position origin;
};

class StateDistribution {
public:
    StateDistribution(std::vector<Board>&& boards) : particles(std::move(boards)) {}
    StateDistribution(StateDistribution&&) = default;
    StateDistribution(const StateDistribution&) = default;

    Board sample() const;

    // Update the board and return the fraction of games won by that move.
    std::tuple<double, std::vector<std::tuple<int, Move, StateDistribution>>>
        update(Move move, Color our_color) const;

    double heuristic_value(Color color) const;

    // Move each particle randomly, splitting into equivalence classes by captured piece
    // Returns [(weight, capture, distribution)]
    std::vector<std::tuple<int, Capture, StateDistribution>>
        update_random(Color opponent_color) const;

    std::vector<Move> get_available_actions(Color color) const;

    // Handle our observation
    void observe(Observation obs, Color our_color);

    // Handle our move
    void handle_move_result(MoveResult move_result, Color our_color);

    // Handle opponent move
    void handle_opponent_move_result(Capture capture, Color opponent_color);

    StateDistribution subsample(size_t num);

    void reinitialize(Board board);

    void entropy(std::array<std::array<double, 8>, 8>& out, Color our_color) const;
    double square_entropy(Position position) const;

private:
    // Move one piece of the given color to some other free spot.
    static Board mutate_board(Board board, Color color);

    std::vector<Board> particles;
};

}

}
