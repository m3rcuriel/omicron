#include <cmath>
#include <algorithm>
#include "chess.h"

namespace chess {

namespace agent {

constexpr size_t kNumParticles = 5000;
constexpr double kUcbGain = 1.0;

struct Particle {
    Board board;
};

class UctTree {};

struct UcbEntry;

double heuristic(const std::array<Particle, kNumParticles>& p);

struct UctNode {
    std::array<Particle, kNumParticles> particles;

    std::vector<UcbEntry> entries_heap;
    std::vector<UctNode> children;

    int num_tries = 0;
    int win_count = 0;

    Color color;

    UctNode(std::array<Particle, kNumParticles> previous, Move move, Color color);
    UctNode(const UctNode& other) = delete;
    UctNode(UctNode&& other) = default;

    double run_uct(int search_depth);
};

struct UcbEntry {
    Move move;
    double total_value;
    int child_index;

    double get_value(int child_tries) const;
    double get_ucb(int parent_tries, int child_tries) const;
};

} // namespace agent

} // namespace chess
