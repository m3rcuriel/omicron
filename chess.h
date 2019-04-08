#include <array>
#include <vector>

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

    bool operator==(const Piece& other) {
        return color == other.color && type == other.type;
    }

    bool operator!=(const Piece& other) {
        return !((*this) == other);
    }
};

struct Position {
    int8_t rank, file;

    Position(int rank, int file) : rank(static_cast<int>(rank)), file(static_cast<int>(file)) {}

    bool operator==(const Position& other) {
        return rank == other.rank && file == other.file;
    }

    bool operator!=(const Position& other) {
        return !((*this) == other);
    }
};

// King's motion (two squares away) represents castling.
// Target is (-1, -1) unless capturing.
struct Move {
    Piece piece;
    Position from, to;
};

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
    Piece apply_move(Move move);

    static Board initial_board();

    void debug_print(std::ostream& out) const;

    Piece get_piece(int i, int j);
    void set_piece(int i, int j, Piece piece);

private:
    // Collect the valid moves for a given piece into the vector at `moves`.
    void collect_moves_for_piece(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_pawn(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_queen(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_king(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_rook(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_knight(int rank, int file, std::vector<Move> *moves) const;
    void collect_moves_for_bishop(int rank, int file, std::vector<Move> *moves) const;

    // Apply a move for a specific piece type.
    Piece apply_move_pawn(Move move);
    Piece apply_move_queen(Move move);
    Piece apply_move_king(Move move);
    Piece apply_move_rook(Move move);
    Piece apply_move_knight(Move move);
    Piece apply_move_bishop(Move move);

    Piece apply_move_linear(Position from, Position to, bool allow_capture);
    Piece move_piece(Position from, Position to);

    Color occupation(int i, int j) const;

    std::array<std::array<Piece, 8>, 8> board;

    bool can_castle_kingside_white = false, can_castle_queenside_white = false,
         can_castle_kingside_black = false, can_castle_queenside_black = false;
    Position en_passant_target {-1, -1};
};

} // namespace chess
