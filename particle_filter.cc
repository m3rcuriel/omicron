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
    return result;
}

std::vector<Move> StateDistribution::get_available_actions(Color color) const {
    return particles[0].generate_moves(color);
}

} // namespace agent

} // namespace chess
