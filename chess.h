#include <array>
#include <vector>
#include <iostream>

namespace chess {

enum class PieceType: uint8_t {
    EMPTY,
    PAWN,
    QUEEN,
    KING,
    ROOK,
    KNIGHT,
    BISHOP
};

enum class Color: uint8_t {
    EMPTY,
    WHITE,
    BLACK
};

#pragma pack()
struct Piece {
    Color color : 2;
    PieceType type : 6;

    static Piece EMPTY;

    char get_symbol() const;

    bool operator==(const Piece& other) const {
        return color == other.color && type == other.type;
    }

    bool operator!=(const Piece& other) const {
        return !((*this) == other);
    }

    bool operator<(const Piece& other) const {
        if (color < other.color) {
            return true;
        } else if (color > other.color) {
            return false;
        } else {
            if (type < other.type) {
                return true;
            } else {
                return false;
            }
        }
    }
};

struct Position {
    int8_t rank, file;

    Position(int rank, int file) : rank(static_cast<int>(rank)), file(static_cast<int>(file)) {}

    static Position NONE;

    bool operator==(const Position& other) const {
        return rank == other.rank && file == other.file;
    }

    bool operator!=(const Position& other) const {
        return !((*this) == other);
    }

    bool operator<(const Position& other) const {
        if (rank < other.rank) {
            return true;
        } else if (other.rank < rank) {
            return false;
        } else {
            return file < other.file;
        }
    }
};

std::ostream& operator<<(std::ostream& out, Position m) {
    out << "(" << (int) m.rank << ", " << (int) m.file << ")";
    return out;
}

struct Capture {
    Piece piece;
    Position position;

    static Capture NONE;

    bool operator==(const Capture& other) const {
        return piece == other.piece && position == other.position;
    }

    bool operator!=(const Capture& other) const {
        return !((*this) == other);
    }

    bool operator<(const Capture& other) const {
        if (piece < other.piece) {
            return true;
        } else if (other.piece < piece) {
            return false;
        } else {
            return position < other.position;
        }
    }
};

// King's motion (two squares away) represents castling.
// Target is (-1, -1) unless capturing.
struct Move {
    Position from, to;
};

std::ostream& operator<<(std::ostream& out, Move m) {
    out << m.from << " -> " << m.to;
    return out;
}

inline Color opponent(Color a) {
    if (a == Color::WHITE) {
        return Color::BLACK;
    } else {
        return Color::WHITE;
    }
}

class Board {
public:
    Board();
    explicit Board(const std::array<std::array<Piece, 8>, 8>& board);
    std::vector<Move> generate_moves(Color turn) const;

    // Apply a move and return the captured piece, if any
    Capture apply_move(Move move);

    static Board initial_board();

    void debug_print(std::ostream& out) const;

    Piece get_piece(int i, int j) const;
    void set_piece(int i, int j, Piece piece);

    bool get_castle_kingside_white() const { return can_castle_kingside_white; }
    bool get_castle_queenside_white() const { return can_castle_queenside_white; }
    bool get_castle_kingside_black() const { return can_castle_kingside_black; }
    bool get_castle_queenside_black() const { return can_castle_queenside_black; }

private:
    // Collect the valid moves for a given piece into the vector at `moves`.
    void collect_moves_for_piece(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_pawn(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_queen(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_king(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_rook(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_knight(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_bishop(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_linear(int rank, int file, int dr, int df, Color color, std::vector<Move> *moves, int d = 8) const;

    // Apply a move for a specific piece type.
    Capture apply_move_pawn(Move move);
    Capture apply_move_queen(Move move);
    Capture apply_move_king(Move move);
    Capture apply_move_rook(Move move);
    Capture apply_move_knight(Move move);
    Capture apply_move_bishop(Move move);

    Capture apply_move_linear(Position from, Position to, bool allow_capture);
    Capture move_piece(Position from, Position to);

    Color occupation(int i, int j) const;

    std::array<std::array<Piece, 8>, 8> board;

    bool can_castle_kingside_white = false, can_castle_queenside_white = false,
         can_castle_kingside_black = false, can_castle_queenside_black = false;
    Position en_passant_target {-1, -1};
};

} // namespace chess
