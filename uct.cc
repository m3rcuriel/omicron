#include "uct.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace chess {

namespace agent {

double heuristic(const std::array<Particle, kNumParticles>&) {
    return 0;
}

Move calculate_opponent_move(Color opponent_color, Board board) {
    static std::default_random_engine generator;
    std::vector possible_moves = board.generate_moves(opponent_color);
    std::uniform_int_distribution<int> move_generator(0, possible_moves.size());
    return possible_moves[move_generator(generator)];
}

UctNode::UctNode(std::array<Particle, kNumParticles> previous, Move move, Color color)
        : color(color) {
    for (size_t i = 0; i < kNumParticles; i++) {
        Particle new_particle = previous[i];
        new_particle.board.apply_move(move);
        // Apply an opponent move
        new_particle.board.apply_move(
                calculate_opponent_move(opponent(color), new_particle.board));
        particles[i] = new_particle;
    }
}

double UctNode::run_uct(int search_depth) {
    std::make_heap(entries_heap.begin(), entries_heap.end(),
        [=](const UcbEntry& a, const UcbEntry& b) {
            return a.get_ucb(num_tries, children[a.child_index].num_tries) <
                b.get_ucb(num_tries, children[b.child_index].num_tries);
        });

    if (search_depth <= 0) {
        return heuristic(particles);
    }

    UcbEntry& current_entry = entries_heap[0];
    if (current_entry.child_index == -1) {
        current_entry.child_index = children.size();
        children.emplace_back(particles, current_entry.move, color);
    }
    double result_value = children[current_entry.child_index].run_uct(search_depth - 1);
    current_entry.total_value += result_value;
    num_tries++;
    return result_value;
}

double UcbEntry::get_value(int child_tries) const {
    return total_value / child_tries;
}

double UcbEntry::get_ucb(int parent_tries, int child_tries) const {
    if (child_tries == 0) {
        return 1;
    }
    return get_value(child_tries) + kUcbGain * std::sqrt(std::log(parent_tries) / child_tries);
}

} // namespace agent

} // namespace chess
