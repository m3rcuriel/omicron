#include "particle_filter.h"
#include "util.h"

namespace chess {

namespace agent {

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
    for (int c = 0; c < 10; c++) {
        const Board& b = random_choice(particles);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece piece = b.get_piece(i, j);
                piece_values += piece_value(piece.type) * color_value(piece.color, color);
            }
        }
    }
    return piece_values / 10.0 / 188.0;
}

std::vector<std::tuple<int, Capture, StateDistribution>>
StateDistribution::update_random(Color opponent_color) const {
    std::vector<std::tuple<int, Capture, StateDistribution>> result;
    std::map<Capture, int> indices;
    for (Board b : particles) {
        Capture capture = b.do_random_move(opponent_color);
        if (indices.find(capture) == indices.end()) {
            indices[capture] = result.size();
            result.push_back(std::make_tuple(1, capture, StateDistribution({b})));
        } else {
            std::get<2>(result[indices[capture]]).particles.push_back(b);
        }
    }
    for (auto& it : result) {
        std::vector<Board>& it_particles = std::get<2>(it).particles;
        while (it_particles.size() < particles.size()) {
            Board b = random_choice(it_particles);
            while (random_float(0, 1) < 0.2) {
                b = mutate_board(b, opponent_color);
            }
            it_particles.push_back(b);
        }
    }
    return result;
}

std::vector<Move> StateDistribution::get_available_actions(Color color) const {
    return particles[0].generate_moves(color);
}

static bool is_valid(const Board& b, Observation obs) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (b.get_piece(obs.origin.rank + i, obs.origin.file + j) != obs.obs[i][j]) {
                return false;
            }
        }
    }
    return true;
}

void StateDistribution::observe(Observation obs, Color our_color) {
    std::vector<Board> result_particles;
    while (result_particles.size() < particles.size()) {
        Board b = random_choice(particles);
        while (random_float(0, 1) < 0.2) {
            b = mutate_board(b, opponent(our_color));
        }
        if (is_valid(b, obs)) {
            result_particles.push_back(b);
        }
    }
    std::swap(particles, result_particles);
}

void StateDistribution::handle_move_result(MoveResult move_result, Color our_color) {
    std::vector<Board> result_particles;
    while (result_particles.size() < particles.size()) {
        Board b = random_choice(particles);
        while (random_float(0, 1) < 0.2) {
            b = mutate_board(b, opponent(our_color));
        }
        Capture capture = b.apply_move(move_result.move);
        if (capture == move_result.capture) {
            result_particles.push_back(b);
        }
    }
    std::swap(particles, result_particles);
}

Board StateDistribution::mutate_board(Board board, Color color) {
    for (int i = 0; i < 10; i++) {
        Board board_copy = board;
        Capture capture = board.do_random_move(color);
        if (capture == Capture::NONE) {
            return board_copy;
        }
    }
    return board;
}

StateDistribution StateDistribution::subsample(size_t num) {
    std::vector<Board> result;
    for (size_t i = 0; i < num; i++) {
        result.push_back(random_choice(particles));
    }
    return StateDistribution(std::move(result));
}

void StateDistribution::reinitialize(Board board) {
    std::vector<Board> new_particles(kNumParticles, board);
    std::swap(new_particles, particles);
}

} // namespace agent

} // namespace chess
