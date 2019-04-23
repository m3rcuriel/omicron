#include "chess_agent.h"

namespace chess {

namespace agent {

std::vector<std::pair<Move, Piece>> white_starting_moves = {
    {Move{{1, 4}, {3, 4}}, Piece{Color::WHITE, PieceType::PAWN}},
    {Move{{0, 5}, {4, 1}}, Piece{Color::WHITE, PieceType::BISHOP}},
    {Move{{4, 1}, {7, 4}}, Piece{Color::WHITE, PieceType::BISHOP}},
    {Move{{6, 3}, {7, 4}}, Piece{Color::WHITE, PieceType::BISHOP}},
};

std::vector<std::pair<Move, Piece>> black_starting_moves = {
    {Move{{6, 4}, {4, 4}}, Piece{Color::BLACK, PieceType::PAWN}},
    {Move{{7, 5}, {3, 1}}, Piece{Color::BLACK, PieceType::BISHOP}},
    {Move{{3, 1}, {0, 4}}, Piece{Color::BLACK, PieceType::BISHOP}},
    {Move{{1, 3}, {0, 4}}, Piece{Color::BLACK, PieceType::BISHOP}},
};

ChessAgent::ChessAgent()
    : particle_filter(
          std::vector<Board>(kNumParticles, Board::initial_board())) {}

void ChessAgent::handle_game_start(Color color) {
  // Reinitialize the particle filter
  our_color = color;
  opening_state = 0;
}

void ChessAgent::handle_opponent_move_result(bool captured_piece,
                                             Position captured_square) {
  particle_filter.handle_opponent_move_result(captured_piece, captured_square,
                                              opponent(our_color));
}

Position ChessAgent::choose_sense(std::vector<Position> possible_sense,
                                  std::vector<Move> possible_moves,
                                  double seconds_left) {
  std::array<std::array<double, 8>, 8> entropies;
  for (auto &r : entropies) {
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

Move ChessAgent::choose_move(double seconds_left) {
  auto& starter_moves = our_color == Color::WHITE
      ? white_starting_moves : black_starting_moves;
  std::cout << opening_state << ", " << starter_moves.size() << std::endl;
  if (opening_state != -1 && opening_state < starter_moves.size()) {
    auto starter_move = starter_moves[opening_state];
    if (particle_filter.particles[0].get_piece(
                starter_move.first.from.rank, starter_move.first.from.file)
            == starter_move.second) {
      opening_state++;
      std::cout << "OPENING MOVE " << opening_state << std::endl;
      return starter_move.first;
    } else {
      std::cout << "OPENING CANCELLED" << std::endl;
      opening_state = -1;
    }
  }

  OurUctNode root(particle_filter.subsample(kNumParticlesRollout), our_color);
  double frac_taken = (600 - seconds_left) / 600.0;
  int rollout_depth = kRolloutDepth * (1 - frac_taken * frac_taken);
  int num_iters = 1000 * (1 - frac_taken * frac_taken);
  for (int i = 0; i < num_iters; i++) {
    root.simulate(rollout_depth);
  }
  return root.find_best_entry().our_move;
}

void ChessAgent::handle_move_result(Move taken_move, bool capture,
                                    Position captured_square) {
  auto& starter_moves = our_color == Color::WHITE
      ? white_starting_moves : black_starting_moves;
  if (opening_state > 0 && opening_state - 1 < starter_moves.size()
          && taken_move.to != starter_moves[opening_state - 1].first.to) {
      std::cout << "OPENING CANCELLED 2" << std::endl;
      opening_state = -1;
  }

  particle_filter.handle_move_result(taken_move, our_color, capture,
                                     captured_square);
}

void ChessAgent::handle_game_end(Color winner_color, std::string reason) {}

}  // namespace agent
}  // namespace chess
