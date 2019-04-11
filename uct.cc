#include "uct.h"

#include <algorithm>
#include <map>
#include <tuple>

namespace chess {

namespace agent {

std::mt19937& get_random_engine() {
    static std::mt19937 mt;
    return mt;
}

template<typename T>
T& random_choice(std::vector<T>& from) {
    auto dist = std::uniform_int_distribution<size_t>(0, from.size() - 1);
    return from[dist(get_random_engine())];
}

template<typename T>
const T& random_choice(const std::vector<T>& from) {
    auto dist = std::uniform_int_distribution<size_t>(0, from.size() - 1);
    return from[dist(get_random_engine())];
}

double random_float(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(get_random_engine());
}

Board StateDistribution::sample() const {
    return random_choice(particles);
}

double StateDistribution::update(Move move) {
    int num_wins = 0;
    int initial_size = particles.size();

    for (size_t i = 0; i < particles.size() - num_wins;) {
        Board& b = particles[i];
        Capture cap = b.apply_move(move);
        if (cap.piece.type == PieceType::KING) {
            num_wins++;
            std::swap(particles[i], particles[particles.size() - num_wins]);
        } else {
            i++;
        }
    }

    particles.erase(particles.end() - num_wins, particles.end());
    return ((double) num_wins) / initial_size;
}

int piece_value(PieceType piece) {
    switch (piece) {
    case PieceType::PAWN:
        return 1;
    case PieceType::KING:
        return 100;
    case PieceType::QUEEN:
        return 20;
    case PieceType::KNIGHT:
        return 10;
    case PieceType::ROOK:
        return 10;
    case PieceType::BISHOP:
        return 10;
    default:
        return 0;
    }
}

int color_value(Color color, Color ours) {
    if (color == ours) {
        return 1;
    } else if (color == opponent(ours)) {
        return -1;
    } else {
        return 0;
    }
}

double StateDistribution::heuristic_value(Color color) {
    double piece_values = 0;
    for (Board& b : particles) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece piece = b.get_piece(i, j);
                piece_values += piece_value(piece.type) * color_value(piece.color, color);
            }
        }
    }
    return piece_values / 40.0 / particles.size();
}

std::vector<std::tuple<int, Capture, StateDistribution>>
StateDistribution::update_random(Color opponent_color) const {
    std::vector<std::tuple<int, Capture, StateDistribution>> result;
    std::map<Capture, int> indices;
    for (Board b : particles) {
        Capture capture = b.apply_move(random_choice(b.generate_moves(opponent_color)));
        if (indices.find(capture) == indices.end()) {
            indices[capture] = result.size();
            result.push_back(std::make_tuple(1, capture, StateDistribution({b})));
        } else {
            std::get<2>(result[indices[capture]]).particles.push_back(b);
        }
    }
    return result;
}

std::vector<Move> StateDistribution::get_available_actions(Color color) const {
    return particles[0].generate_moves(color);
}

OurUctNode::OurUctNode(Board board, Color color)
        : OurUctNode(
                StateDistribution(std::vector<Board>(kNumParticles, board)),
                color) {}

OurUctNode::OurUctNode(StateDistribution state, Color color)
        : state(state), color(color) {
    // Calculate the list of moves.
    for (Move m : state.get_available_actions(color)) {
        ucb_table.emplace_back(state, m, color);
        ucb_table.back().value += random_float(-1e-3, 1e-3);
    }

    // TODO(Kyle): Check that all boards have the same pieces for us.
    // TODO(Kyle): Assert that all boards have the same moves for us.
}

double calculate_ucb(double value, double n, double N) {
    if (n == 0) {
        return std::numeric_limits<double>::infinity();
    }
    return value + 10 * std::sqrt(std::log(N) / n);
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

    double reward = best_entry.reward;
    if (reward < 1 - 1e-10) {
        reward += 0.95 * (1 - reward) * best_entry.node->simulate(depth);
    }

    count++;
    best_entry.count++;
    best_entry.value = best_entry.value + (reward - best_entry.value) / best_entry.count;
    return reward;
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
        reward += h;
    }
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
    double R = reward + child.simulate(depth - 1);
    count++;
    value = value + (R - value) / count;
    return value;
}

} // namespace agent

} // namespace chess
