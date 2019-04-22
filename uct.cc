#include "uct.h"

#include <algorithm>
#include <map>
#include <tuple>
#include <random>

#include "util.h"

namespace chess {

namespace agent {

std::mt19937& get_random_engine() {
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
    for (UcbEntry& entry : ucb_table) {
        std::cout << entry.our_move << " with value " << entry.value << " and UCB " << calculate_ucb(entry.value, entry.count, count) << std::endl;
    }
}

double OurUctNode::simulate(int depth) {
    if (depth < 0) {
        return 0;
    }

    UcbEntry& best_entry = find_best_entry();

    double reward = best_entry.value;

    if (reward < 1 - 1e-10) {
        double sim_reward = best_entry.node->simulate(depth);
        reward += 0.95 * (1 - reward) * sim_reward;
    }

    count++;
    best_entry.count++;
    best_entry.value += (reward - best_entry.value) / best_entry.count;
    return reward;
}

UcbEntry& OurUctNode::find_best_entry() {
    // Find the largest UCB entry, by UCB.
    UcbEntry& best_entry = *std::max_element(
        ucb_table.begin(), ucb_table.end(),
        [&](const UcbEntry& a, const UcbEntry& b) {
            return calculate_ucb(a.value, a.count, count) <
                calculate_ucb(b.value, b.count, count);
        }
    );

    if (!best_entry.node) {
        best_entry.generate();
    }

    return best_entry;
}

UcbEntry::UcbEntry(const StateDistribution& state_prior, Move our_move, Color our_color)
        : our_color(our_color),
          our_move(our_move),
          node(nullptr),
          state(state_prior),
          count(0) {
    reward = state.update(our_move);

    if (reward < 1 - 1e-10) {
        double h = state.heuristic_value(our_color);
        reward += h * (1 - reward);
        count = 2;
    }

    value = reward;
}

void UcbEntry::generate() {
    node = std::unique_ptr<OpponentUctNode>(new OpponentUctNode(state, our_color));
    if (!node) {
        abort();
    }
}

OpponentUctNode::OpponentUctNode(const StateDistribution& state_prior, Color our_color)
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

    child_weights = std::discrete_distribution<int>(weights.begin(), weights.end());
}

double OpponentUctNode::simulate(int depth) {
    OurUctNode& child = children[child_weights(get_random_engine())];
    double R = reward + (1 + reward) * child.simulate(depth - 1);
    count++;
    value += (R - value) / count;
    return value;
}

} // namespace agent

} // namespace chess
