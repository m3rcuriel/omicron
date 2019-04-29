#include <memory>
#include <random>
#include <vector>

#include "chess.h"
#include "particle_filter.h"

namespace chess {

namespace agent {

constexpr double kUcbConstant = 1;

class OurUctNode;
class OpponentUctNode;
struct UcbEntry;

// A game state represented by a set of particles.
// Corresponds to T(ha).
class OurUctNode {
 public:
  OurUctNode(Board board, Color color);
  OurUctNode(StateDistribution state, Color color);

  void print_moves();

  double simulate(int depth);

  double get_value() const;

  UcbEntry &find_best_entry(double uct_constant = kUcbConstant);

 private:
  // The state distribution before our move.
  StateDistribution state;

  Color color;
  std::vector<UcbEntry> ucb_table;

  int count = 0;
};

// A set of particles plus the next action to be taken by us.
// Corresponds to T(ha).
struct UcbEntry {
  UcbEntry(const StateDistribution &state_prior, Move our_move,
           Color our_color);
  ~UcbEntry();

  double simulate(int depth);

  void generate();

  // Create the opponent nodes.
  // void generate();

  Color our_color;

  // The corresponding move
  Move our_move;

  // The opponent's UCT nodes are initialized in a lazy fashion
  // to save on RAM usage.
  std::vector<std::pair<OpponentUctNode *, StateDistribution>> children;
  std::discrete_distribution<int> child_weights;

  // The number of times this entry has been taken.
  int count = 0;

  // The expected value of this entry.
  double value = 0;

  // The immediate reward for having taken this action.
  double reward = 0;
};

// Allows us to split the tree on opponent moves. This is not
// included in the paper on POMCPs.
class OpponentUctNode {
 public:
  OpponentUctNode(const StateDistribution &state_prior, Color our_color);

  // Returns the reward from a single simulated instance.
  double simulate(int depth);

 private:
  Color our_color;

  // Immediate reward, calculated from wins - losses in initial particles.
  double reward = 0;

  double value = 0;

  // The total number of times this node has been explored.
  // Immediately initialized to the number of wins + losses
  // (those games are assumed to be simulated out fully).
  int count = 0;

  std::vector<OurUctNode> children;

  std::discrete_distribution<int> child_weights;

  // The state before the opponent move.
  StateDistribution state;
};

}  // namespace agent

}  // namespace chess
