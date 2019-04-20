#include <pybind11/pybind11.h>
#include <random>

#include "particle_filter.h"
#include "uct.h"
#include "chess.h"

namespace py = pybind11;

namespace chess {

namespace agent {

constexpr int kRolloutDepth = 10;

class ChessAgent {
 public:
  ChessAgent();

  void handle_game_start(Color color, Board board);
  void handle_opponent_move_result(Piece captured_piece,
                                   Position captured_square);

  Position choose_sense(std::vector<Position> possible_sense,
                        std::vector<Move> possible_moves,
                        double seconds_left);

  void handle_sense_result(Observation sense_result);

  Move choose_move(std::vector<Move> possible_moves, double seconds_left);

  void handle_move_result(Move requested_move, Move taken_move,
                          std::string reason, Piece captured_piece,
                          Position captured_square);

  void handle_game_end(Color winner_color, std::string reason);

 private:
  std::default_random_engine generator;
  StateDistribution particle_filter;
  Color our_color;
};

} // namespace agent

} // namespace chess
