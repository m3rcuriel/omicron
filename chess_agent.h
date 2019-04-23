#include <random>

#include "chess.h"
#include "particle_filter.h"
#include "uct.h"

namespace chess {

namespace agent {

constexpr int kRolloutDepth = 5;

class ChessAgent {
 public:
  ChessAgent();

  void handle_game_start(Color color);
  void handle_opponent_move_result(bool captured_piece,
                                   Position captured_square);

  Position choose_sense(std::vector<Position> possible_sense,
                        std::vector<Move> possible_moves, double seconds_left);

  void handle_sense_result(Observation sense_result);

  Move choose_move(double seconds_left);

  void handle_move_result(Move taken_move, bool capture,
                          Position captured_square);

  void handle_game_end(Color winner_color, std::string reason);

 private:
  std::default_random_engine generator;
  StateDistribution particle_filter;
  Color our_color;
};

}  // namespace agent

}  // namespace chess
