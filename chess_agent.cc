#include "chess_agent.h"

namespace chess {

namespace agent {

ChessAgent::ChessAgent() : particle_filter(
        std::vector<Board>(
            kNumParticles, Board::initial_board())) {}

void ChessAgent::handle_game_start(Color color, Board board) {
    // Reinitialize the particle filter
    particle_filter.reinitialize(board);
    our_color = color;
}

void ChessAgent::handle_opponent_move_result(
        Piece captured_piece,
        Position captured_square) {
    particle_filter.handle_opponent_move_result(
            {captured_piece, captured_square}, opponent(our_color));
}

Position ChessAgent::choose_sense(
        std::vector<Position> possible_sense,
        std::vector<Move> possible_moves,
        double seconds_left) {
    std::array<std::array<double, 8>, 8> entropies;
    for (auto& r : entropies) {
        r.fill(0);
    }

    particle_filter.entropy(entropies, our_color);

    double max_entropy_sum = 0.0;
    // Track the best top-left square
    Position max_entropy_position = Position::NONE;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            double total_entropy = 0;
            for (int ii = 0; ii < 3; ii++) {
                for (int jj = 0; jj < 3; jj++) {
                    total_entropy += entropies[i + ii][j + jj];
                }
            }
            if (total_entropy > max_entropy_sum) {
                max_entropy_sum = total_entropy;
                max_entropy_position = {i, j};
            }
        }
    }

    // Convert to center square
    return {max_entropy_position.rank + 1, max_entropy_position.file + 1};
}

void ChessAgent::handle_sense_result(Observation sense_result) {
    particle_filter.observe(sense_result, our_color);
}

Move ChessAgent::choose_move(
        std::vector<Move> possible_moves, double seconds_left) {
    OurUctNode root(
            particle_filter.subsample(kNumParticlesRollout), our_color);
    for (int i = 0; i < 1000; i++) {
        root.simulate(kRolloutDepth);
    }
    return root.find_best_entry().our_move;
}

void ChessAgent::handle_move_result(
        Move requested_move, Move taken_move,
        std::string reason, Piece captured_piece,
        Position captured_square) {
    particle_filter.handle_move_result({taken_move, captured_piece}, our_color);
}

void ChessAgent::handle_game_end(Color winner_color, std::string reason) {}

} // namespace agent

} // namespace chess
