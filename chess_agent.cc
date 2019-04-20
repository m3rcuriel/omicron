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
        Position captured_square) {}

Position ChessAgent::choose_sense(
        std::vector<Position> possible_sense,
        std::vector<Move> possible_moves, double seconds_left) {
    // TODO Find the best sense
    return possible_sense[0];
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
        Position captured_square) {}

void ChessAgent::handle_game_end(Color winner_color, std::string reason) {}

} // namespace agent

} // namespace chess
