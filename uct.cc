#include "uct.h"

#include <algorithm>
#include <map>
#include <random>
#include <tuple>

#include "util.h"

namespace chess {

namespace agent {

std::mt19937 &get_random_engine() {
  static std::mt19937 mt;
  return mt;
}

OurUctNode::OurUctNode(Board board, Color color)
    : OurUctNode(
          StateDistribution(std::vector<Board>(kNumParticlesRollout, board)),
          color) {}

OurUctNode::OurUctNode(StateDistribution state, Color color)
    : state(state), color(color) {
  // Calculate the list of moves.
  state.CheckValid(color);
  for (Move m : state.get_available_actions(color)) {
    ucb_table.emplace_back(state, m, color);
    ucb_table.back().value += random_float(-1e-200, 1e-200);
    count += 2;
  }

  // TODO(Kyle): Check that all boards have the same pieces for us.
  // TODO(Kyle): Assert that all boards have the same moves for us.
}

double calculate_ucb(double value, double n, double N) {
  if (n == 0) {
    return std::numeric_limits<double>::infinity();
  }
  return value + kUcbConstant * std::sqrt(std::log(N) / n);
}

void OurUctNode::print_moves() {
  for (UcbEntry &entry : ucb_table) {
    std::cout << entry.our_move << " with value " << entry.value << " and UCB "
              << calculate_ucb(entry.value, entry.count, count) << std::endl;
  }
}

double OurUctNode::simulate(int depth) {
  if (depth < 0) {
    return 0;
  }

  UcbEntry &best_entry = find_best_entry();

  count++;
  return best_entry.simulate(depth);
}

UcbEntry &OurUctNode::find_best_entry() {
  // Find the largest UCB entry, by UCB.
  UcbEntry &best_entry =
      *std::max_element(ucb_table.begin(), ucb_table.end(),
                        [&](const UcbEntry &a, const UcbEntry &b) {
                          return calculate_ucb(a.value, a.count, count) <
                                 calculate_ucb(b.value, b.count, count);
                        });

  return best_entry;
}

UcbEntry::UcbEntry(const StateDistribution &state_prior, Move our_move,
                   Color our_color)
    : our_color(our_color), our_move(our_move), count(0) {
  std::vector<std::tuple<int, Move, StateDistribution>> result_dists;
  std::tie(reward, result_dists) = state_prior.update(our_move, our_color);

  double reward_heuristic = 0;

  std::vector<double> weights;
  for (auto &d : result_dists) {
    auto thingy = std::get<2>(d);
    std::get<2>(d).CheckValid(our_color);
    children.push_back(std::make_pair(nullptr, std::get<2>(d)));
    weights.push_back(std::get<0>(d));

    double h = std::get<2>(d).heuristic_value(our_color);
    reward_heuristic += h * std::get<0>(d) * (1 - reward);
  }

  reward += reward_heuristic;
  count = 2;

  value = reward;

  child_weights =
      std::discrete_distribution<int>(weights.begin(), weights.end());
}

double UcbEntry::simulate(int depth) {
  double reward = value;

  int idx = child_weights(get_random_engine());
  auto &node = children[idx];
  if (node.first == nullptr) {
    node.first = new OpponentUctNode(node.second, our_color);
  }

  if (reward < 1 - 1e-10) {
    double sim_reward = node.first->simulate(depth);
    reward += 0.95 * (1 - reward) * sim_reward;
  }

  count++;
  value += (reward - value) / count;
  return value;
}

void UcbEntry::generate() {}

UcbEntry::~UcbEntry() {
  for (auto &c : children) {
    if (c.first) {
      delete c.first;
    }
  }
}

OpponentUctNode::OpponentUctNode(const StateDistribution &state_prior,
                                 Color our_color)
    : state(state_prior) {
  int total_count = 0;
  std::vector<double> weights;

  for (auto t : state_prior.update_random(opponent(our_color))) {
    int count = std::get<0>(t);
    Capture capture = std::get<1>(t);

    total_count += count;
    if (capture.piece.type == PieceType::KING) {
      reward -= count;
    } else {
      StateDistribution dist = std::get<2>(t);
      children.push_back(OurUctNode(state, our_color));
      weights.push_back(count);
    }
  }

  reward /= total_count;

  value = reward;

  child_weights =
      std::discrete_distribution<int>(weights.begin(), weights.end());
}

double OpponentUctNode::simulate(int depth) {
  OurUctNode &child = children[child_weights(get_random_engine())];
  double R = reward + (1 + reward) * child.simulate(depth - 1);
  count++;
  value += (R - value) / count;
  return value;
}

}  // namespace agent

}  // namespace chess
