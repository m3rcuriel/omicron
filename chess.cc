#include "chess.h"
#include <iostream>
#include <cassert>

namespace chess {

Piece Piece::EMPTY{Color::EMPTY, PieceType::EMPTY};

namespace {

int three_way_compare(int a, int b) {
    return a > b ? 1 : (b > a ? -1 : 0);
}

int mirrored_rank(Color color, int rank) {
    if (color == Color::WHITE) {
        return rank;
    } else {
        return 7 - rank;
    }
}

void add_move(
        Piece piece,
        int r, int f,
        int nr, int nf,
        std::vector<Move> *moves,
        int tr = -1, int tf = -1) {
    if (nr < 0 || nr >= 8 || nf < 0 || nf >= 8) {
        return;
    }
    if (tr == -1 && tf == -1) {
        tr = nr;
        tf = nf;
    }
    moves->push_back(Move{piece, Position{r, f}, Position{nr, nf}});
}

}

Board::Board() {
    for (std::array<Piece, 8>& row : board) {
        row.fill(Piece{Color::EMPTY, PieceType::EMPTY});
    }
}

Board::Board(const std::array<std::array<Piece, 8>, 8>& board) : board(board) {}

std::vector<Move> Board::generate_moves(Color turn) const {
    std::vector<Move> result;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j].color == turn) {
                collect_moves_for_piece(i, j, &result);
            }
        }
    }
    return result;
}

Piece Board::get_piece(int i, int j) {
    return board[i][j];
}

void Board::set_piece(int i, int j, Piece piece) {
    board[i][j] = piece;
}

Board Board::initial_board() {
    Board result;

    /*   0 1 2 3 4 5 6 7
     * 7|r n b q k b n r
     * 6|p p p p p p p p
     * 5|
     * 4|
     * 3|
     * 2|
     * 1|P P P P P P P P
     * 0|R N B Q K B N R
     */
    result.board[7][0] = Piece{Color::BLACK, PieceType::ROOK};
    result.board[7][1] = Piece{Color::BLACK, PieceType::KNIGHT};
    result.board[7][2] = Piece{Color::BLACK, PieceType::BISHOP};
    result.board[7][3] = Piece{Color::BLACK, PieceType::QUEEN};
    result.board[7][4] = Piece{Color::BLACK, PieceType::KING};
    result.board[7][5] = Piece{Color::BLACK, PieceType::BISHOP};
    result.board[7][6] = Piece{Color::BLACK, PieceType::KNIGHT};
    result.board[7][7] = Piece{Color::BLACK, PieceType::ROOK};
    result.board[6][0] = Piece{Color::BLACK, PieceType::PAWN};
    result.board[6][1] = Piece{Color::BLACK, PieceType::PAWN};
    result.board[6][2] = Piece{Color::BLACK, PieceType::PAWN};
    result.board[6][3] = Piece{Color::BLACK, PieceType::PAWN};
    result.board[6][4] = Piece{Color::BLACK, PieceType::PAWN};
    result.board[6][5] = Piece{Color::BLACK, PieceType::PAWN};
    result.board[6][6] = Piece{Color::BLACK, PieceType::PAWN};
    result.board[6][7] = Piece{Color::BLACK, PieceType::PAWN};

    result.board[0][0] = Piece{Color::WHITE, PieceType::ROOK};
    result.board[0][1] = Piece{Color::WHITE, PieceType::KNIGHT};
    result.board[0][2] = Piece{Color::WHITE, PieceType::BISHOP};
    result.board[0][3] = Piece{Color::WHITE, PieceType::QUEEN};
    result.board[0][4] = Piece{Color::WHITE, PieceType::KING};
    result.board[0][5] = Piece{Color::WHITE, PieceType::BISHOP};
    result.board[0][6] = Piece{Color::WHITE, PieceType::KNIGHT};
    result.board[0][7] = Piece{Color::WHITE, PieceType::ROOK};
    result.board[1][0] = Piece{Color::WHITE, PieceType::PAWN};
    result.board[1][1] = Piece{Color::WHITE, PieceType::PAWN};
    result.board[1][2] = Piece{Color::WHITE, PieceType::PAWN};
    result.board[1][3] = Piece{Color::WHITE, PieceType::PAWN};
    result.board[1][4] = Piece{Color::WHITE, PieceType::PAWN};
    result.board[1][5] = Piece{Color::WHITE, PieceType::PAWN};
    result.board[1][6] = Piece{Color::WHITE, PieceType::PAWN};
    result.board[1][7] = Piece{Color::WHITE, PieceType::PAWN};

    result.can_castle_kingside_white = true;
    result.can_castle_queenside_white = true;
    result.can_castle_kingside_black = true;
    result.can_castle_queenside_black = true;

    return result;
}

void Board::debug_print(std::ostream& out) const {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            out.put(board[i][j].get_symbol());
            if (j != 7) {
                out.put(' ');
            }
        }
        out.put('\n');
    }
}

Piece Board::apply_move(Move move) {
    switch (move.piece.type) {
        case PieceType::KING:
            return apply_move_king(move);
        case PieceType::QUEEN:
            return apply_move_queen(move);
        case PieceType::ROOK:
            return apply_move_rook(move);
        case PieceType::BISHOP:
            return apply_move_bishop(move);
        case PieceType::PAWN:
            return apply_move_pawn(move);
        case PieceType::KNIGHT:
            return apply_move_knight(move);
        default:
            assert(false);
            return Piece::EMPTY;
    }
}

Piece Board::apply_move_pawn(Move move) {
    Color color = move.piece.color;
    int mirrored_from_rank = mirrored_rank(color, move.from.rank),
        mirrored_to_rank = mirrored_rank(color, move.to.rank);
    if (move.from.file == move.to.file) {
        assert(mirrored_to_rank == mirrored_from_rank + 1
                || mirrored_to_rank == mirrored_from_rank + 2);
        assert(mirrored_from_rank == 1);
        apply_move_linear(move.from, move.to, false);
        if (mirrored_to_rank == mirrored_from_rank + 2) {
            en_passant_target = {
                mirrored_rank(color, mirrored_from_rank + 1),
                move.from.file
            };
        }
        return Piece::EMPTY;
    } else {
        assert(abs(move.to.file - move.from.file) == 1);
        assert(mirrored_to_rank == mirrored_from_rank + 1);
        if (occupation(move.to.rank, move.to.file) == opponent(color)) {
            // Regular capture
            return move_piece(move.from, move.to);
        } else if (en_passant_target.rank >= 0) {
            // En passant
            assert(occupation(move.to.rank, move.to.file) != color);
            assert(move.to.file == en_passant_target.file);
            assert(mirrored_to_rank == mirrored_rank(color, en_passant_target.rank));
            int captured_target_rank = mirrored_rank(color, mirrored_to_rank - 1);
            int captured_target_file = en_passant_target.file;
            Piece captured = move_piece(move.from, move.to);
            assert(captured == Piece::EMPTY);
            std::swap(board[captured_target_rank][captured_target_file], captured);
            return captured;
        } else {
            // Wasted move
            return Piece::EMPTY;
        }
    }
}

Piece Board::apply_move_queen(Move move) {
    return apply_move_linear(move.from, move.to, true);
}

Piece Board::apply_move_king(Move move) {
    Color color = move.piece.color;
    int mirrored_from_rank = mirrored_rank(color, move.from.rank),
        mirrored_to_rank = mirrored_rank(color, move.to.rank);
    bool &can_castle_kingside = (color == Color::WHITE ? can_castle_kingside_white : can_castle_kingside_black);
    bool &can_castle_queenside = (color == Color::WHITE ? can_castle_queenside_white : can_castle_queenside_black);
    if (abs(move.from.file - move.to.file) > 1) {
        assert(mirrored_from_rank == mirrored_to_rank);
        assert(mirrored_from_rank == 0);
        assert(move.from.file == 4);
        if (move.from.file < move.to.file) {
            // Kingside
            assert(can_castle_kingside);
            if (occupation(move.from.rank, 6) != Color::EMPTY
                    || occupation(move.from.rank, 5) != Color::EMPTY) {
                // We can't take this move, so don't revoke castling ability.
                return Piece::EMPTY;
            }
            move_piece(move.from, move.to);
            move_piece({move.from.rank, 7}, {move.to.rank, 5});
        } else {
            // Queenside
            assert(can_castle_queenside);
            if (occupation(move.from.rank, 1) != Color::EMPTY
                    || occupation(move.from.rank, 2) != Color::EMPTY
                    || occupation(move.from.rank, 3) != Color::EMPTY) {
                // Same situation as above: no move taken
                return Piece::EMPTY;
            }
            move_piece(move.from, move.to);
            move_piece({move.from.rank, 0}, {move.to.rank, 3});
        }
        can_castle_kingside = can_castle_queenside = false;
        return Piece::EMPTY;
    } else {
        assert(std::max(
                    abs(move.to.rank - move.from.rank),
                    abs(move.to.file - move.from.file)) == 1);
        assert(occupation(move.to.rank, move.to.file) != color);
        can_castle_kingside = can_castle_queenside = false;
        return move_piece(move.from, move.to);
    }
}

Piece Board::apply_move_rook(Move move) {
    assert((move.to.rank - move.from.rank) == 0 || (move.to.file - move.from.file) == 0);
    Color color = move.piece.color;
    int mirrored_from_rank = mirrored_rank(color, move.from.rank);
    bool &can_castle_kingside = (color == Color::WHITE ? can_castle_kingside_white : can_castle_kingside_black);
    bool &can_castle_queenside = (color == Color::WHITE ? can_castle_queenside_white : can_castle_queenside_black);
    if (move.from.file == 7 && mirrored_from_rank == 0) {
        can_castle_kingside = false;
    } else if (move.from.file == 0 && mirrored_from_rank == 0) {
        can_castle_queenside = false;
    }
    return apply_move_linear(move.from, move.to, true);
}

Piece Board::apply_move_knight(Move move) {
    assert(abs(move.to.rank - move.from.rank) == 2 || abs(move.to.file - move.from.file) == 2);
    assert(abs(move.to.rank - move.from.rank) == 1 || abs(move.to.file - move.from.file) == 1);
    assert(occupation(move.to.rank, move.to.file) != move.piece.color);
    return move_piece(move.from, move.to);
}

Piece Board::apply_move_bishop(Move move) {
    assert(abs(move.to.rank - move.from.rank) == abs(move.to.file - move.from.file));
    return apply_move_linear(move.from, move.to, true);
}

Piece Board::apply_move_linear(Position from, Position to, bool allow_capture) {
    int drank = three_way_compare(to.rank, from.rank);
    int dfile = three_way_compare(to.file, from.file);
    Color color = board[from.rank][from.file].color;
    assert((to.rank - from.rank) * dfile == (to.file - from.file) * drank);

    int rank = from.rank, file = from.file;
    while (rank != to.rank) {
        if (occupation(rank + drank, file + dfile) == color) {
            break;
        } else if (occupation(rank + drank, file + dfile) == opponent(color)) {
            if (allow_capture) {
                rank += drank;
                file += dfile;
            }
            break;
        }
        rank += drank;
        file += dfile;
    }
    return move_piece(from, Position{rank, file});
}

Piece Board::move_piece(Position from, Position to) {
    Piece captured = board[to.rank][to.file];
    board[to.rank][to.file] = board[from.rank][from.file];
    board[from.rank][from.file] = Piece::EMPTY;
    en_passant_target = {-1, -1};
    return captured;
}

void Board::collect_moves_for_piece(int rank, int file, std::vector<Move> *moves) const {
    switch (board[rank][file].type) {
        case PieceType::PAWN:
            collect_moves_for_pawn(rank, file, moves);
            break;
        case PieceType::QUEEN:
            collect_moves_for_queen(rank, file, moves);
            break;
        case PieceType::KING:
            collect_moves_for_king(rank, file, moves);
            break;
        case PieceType::ROOK:
            collect_moves_for_rook(rank, file, moves);
            break;
        case PieceType::KNIGHT:
            collect_moves_for_knight(rank, file, moves);
            break;
        case PieceType::BISHOP:
            collect_moves_for_bishop(rank, file, moves);
            break;
        default:
            break;
    }
}

Color Board::occupation(int i, int j) const {
    return board[i][j].color;
}

void Board::collect_moves_for_pawn(int rank, int file, std::vector<Move> *moves) const {
    Piece piece = board[rank][file];
    Color color = piece.color;
    int8_t direction = (color == Color::BLACK) ? -1 : 1;
    if (occupation(rank + direction, file) == Color::EMPTY) {
        add_move(piece, rank, file, rank + direction, file, moves);

        if (((color == Color::BLACK && rank == 6)
                || (color == Color::WHITE && rank == 1))
                && occupation(rank + direction * 2, file) == Color::EMPTY) {
            add_move(piece, rank, file, rank + direction * 2, file, moves);
        }
    }

    if (occupation(rank + direction, file + 1) == opponent(color)) {
        add_move(piece, rank, file, rank + direction, file + 1, moves);
    }
    if (occupation(rank + direction, file - 1) == opponent(color)) {
        add_move(piece, rank, file, rank + direction, file - 1, moves);
    }

    // TODO En passant capture
    if (en_passant_target.rank == rank + direction
            && abs(en_passant_target.file - file) == 1) {
        add_move(piece, rank, file, en_passant_target.rank, en_passant_target.file, moves);
    }
}

void Board::collect_moves_for_queen(int rank, int file, std::vector<Move> *moves) const {
    Piece piece = board[rank][file];
    Color color = piece.color;

    // Up
    for (int i = rank + 1; i < 8; i++) {
        if (occupation(i, file) != color) {
            add_move(piece, rank, file, i, file, moves);
        }
        if (occupation(i, file) != Color::EMPTY) {
            break;
        }
    }

    // Down
    for (int i = rank - 1; i >= 0; i--) {
        if (occupation(i, file) != color) {
            add_move(piece, rank, file, i, file, moves);
        }
        if (occupation(i, file) != Color::EMPTY) {
            break;
        }
    }

    // Left
    for (int i = file + 1; i < 8; i++) {
        if (occupation(rank, i) != color) {
            add_move(piece, rank, file, rank, i, moves);
        }
        if (occupation(rank, i) != Color::EMPTY) {
            break;
        }
    }

    // Right
    for (int i = file - 1; i >= 0; i--) {
        if (occupation(rank, i) != color) {
            add_move(piece, rank, file, rank, i, moves);
        }
        if (occupation(rank, i) != Color::EMPTY) {
            break;
        }
    }

    // Diag up-right
    for (int i = 1; rank + i < 8 && file + i < 8; i++) {
        if (occupation(rank + i, file + i) != color) {
            add_move(piece, rank, file, rank + i, file + i, moves);
        }
        if (occupation(rank + i, file + i) != Color::EMPTY) {
            break;
        }
    }

    // Diag up-left
    for (int i = 1; rank + i < 8 && file - i >= 0; i++) {
        if (occupation(rank + i, file - i) != color) {
            add_move(piece, rank, file, rank + i, file - i, moves);
        }
        if (occupation(rank + i, file - i) != Color::EMPTY) {
            break;
        }
    }

    // Diag down-left
    for (int i = 1; rank - i >= 0 && file - i >= 0; i++) {
        if (occupation(rank - i, file - i) != color) {
            add_move(piece, rank, file, rank - i, file - i, moves);
        }
        if (occupation(rank + i, file - i) != Color::EMPTY) {
            break;
        }
    }

    // Diag down-right
    for (int i = 1; rank - i >= 0 && file + i < 8; i++) {
        if (occupation(rank - i, file + i) != color) {
            add_move(piece, rank, file, rank - i, file + i, moves);
        }
        if (occupation(rank - i, file + i) != Color::EMPTY) {
            break;
        }
    }
}

void Board::collect_moves_for_king(int rank, int file, std::vector<Move> *moves) const {
    Piece piece = board[rank][file];
    Color color = piece.color;

    // Up
    if (occupation(rank + 1, file) != color) {
        add_move(piece, rank, file, rank + 1, file, moves);
    }

    // Down
    if (occupation(rank - 1, file) != color) {
        add_move(piece, rank, file, rank - 1, file, moves);
    }

    // Left
    if (occupation(rank, file - 1) != color) {
        add_move(piece, rank, file, rank, file - 1, moves);
    }

    // Right
    if (occupation(rank, file + 1) != color) {
        add_move(piece, rank, file, rank, file + 1, moves);
    }

    // Diag up-right
    if (occupation(rank + 1, file + 1) != color) {
        add_move(piece, rank, file, rank + 1, file + 1, moves);
    }

    // Diag up-left
    if (occupation(rank + 1, file - 1) != color) {
        add_move(piece, rank, file, rank + 1, file - 1, moves);
    }

    // Diag down-left
    if (occupation(rank - 1, file - 1) != color) {
        add_move(piece, rank, file, rank - 1, file - 1, moves);
    }

    // Diag down-right
    if (occupation(rank - 1, file + 1) != color) {
        add_move(piece, rank, file, rank - 1, file + 1, moves);
    }

    bool can_castle_kingside = (color == Color::WHITE ? can_castle_kingside_white : can_castle_kingside_black);
    bool can_castle_queenside = (color == Color::WHITE ? can_castle_queenside_white : can_castle_queenside_black);

    // Castling
    if (can_castle_kingside
            && occupation(rank, 5) == Color::EMPTY
            && occupation(rank, 6) == Color::EMPTY) {
        add_move(piece, rank, file, rank, 6, moves);
    }
    if (can_castle_queenside
            && occupation(rank, 2) == Color::EMPTY
            && occupation(rank, 3) == Color::EMPTY) {
        add_move(piece, rank, file, rank, 2, moves);
    }
}

void Board::collect_moves_for_rook(int rank, int file, std::vector<Move> *moves) const {
    Piece piece = board[rank][file];
    Color color = piece.color;

    // Up
    for (int i = rank + 1; i < 8; i++) {
        if (occupation(i, file) != color) {
            add_move(piece, rank, file, i, file, moves);
        }
        if (occupation(i, file) != Color::EMPTY) {
            break;
        }
    }

    // Down
    for (int i = rank - 1; i >= 0; i--) {
        if (occupation(i, file) != color) {
            add_move(piece, rank, file, i, file, moves);
        }
        if (occupation(i, file) != Color::EMPTY) {
            break;
        }
    }

    // Left
    for (int i = file + 1; i < 8; i++) {
        if (occupation(rank, i) != color) {
            add_move(piece, rank, file, rank, i, moves);
        }
        if (occupation(rank, i) != Color::EMPTY) {
            break;
        }
    }

    // Right
    for (int i = file - 1; i >= 0; i--) {
        if (occupation(rank, i) != color) {
            add_move(piece, rank, file, rank, i, moves);
        }
        if (occupation(rank, i) != Color::EMPTY) {
            break;
        }
    }
}

void Board::collect_moves_for_knight(int rank, int file, std::vector<Move> *moves) const {
    Piece piece = board[rank][file];
    Color color = piece.color;

    if (occupation(rank + 2, file + 1) != color) {
        add_move(piece, rank, file, rank + 2, file + 1, moves);
    }
    if (occupation(rank + 2, file - 1) != color) {
        add_move(piece, rank, file, rank + 2, file - 1, moves);
    }
    if (occupation(rank - 2, file + 1) != color) {
        add_move(piece, rank, file, rank - 2, file + 1, moves);
    }
    if (occupation(rank - 2, file - 1) != color) {
        add_move(piece, rank, file, rank - 2, file - 1, moves);
    }
    if (occupation(rank + 1, file + 2) != color) {
        add_move(piece, rank, file, rank + 1, file + 2, moves);
    }
    if (occupation(rank + 1, file - 2) != color) {
        add_move(piece, rank, file, rank + 1, file - 2, moves);
    }
    if (occupation(rank - 1, file + 2) != color) {
        add_move(piece, rank, file, rank - 1, file + 2, moves);
    }
    if (occupation(rank - 1, file - 2) != color) {
        add_move(piece, rank, file, rank - 1, file - 2, moves);
    }
}

void Board::collect_moves_for_bishop(int rank, int file, std::vector<Move> *moves) const {
    Piece piece = board[rank][file];
    Color color = piece.color;

    // Diag up-right
    for (int i = 1; rank + i < 8 && file + i < 8; i++) {
        if (occupation(rank + i, file + i) != color) {
            add_move(piece, rank, file, rank + i, file + i, moves);
        }
        if (occupation(rank + i, file + i) != Color::EMPTY) {
            break;
        }
    }

    // Diag up-left
    for (int i = 1; rank + i < 8 && file - i >= 0; i++) {
        if (occupation(rank + i, file - i) != color) {
            add_move(piece, rank, file, rank + i, file - i, moves);
        }
        if (occupation(rank + i, file - i) != Color::EMPTY) {
            break;
        }
    }

    // Diag down-left
    for (int i = 1; rank - i >= 0 && file - i >= 0; i++) {
        if (occupation(rank - i, file - i) != color) {
            add_move(piece, rank, file, rank - i, file - i, moves);
        }
        if (occupation(rank + i, file - i) != Color::EMPTY) {
            break;
        }
    }

    // Diag down-right
    for (int i = 1; rank - i >= 0 && file + i < 8; i++) {
        if (occupation(rank - i, file + i) != color) {
            add_move(piece, rank, file, rank - i, file + i, moves);
        }
        if (occupation(rank - i, file + i) != Color::EMPTY) {
            break;
        }
    }
}

char Piece::get_symbol() const {
    char symbol = ' ';
    switch (type) {
        case PieceType::PAWN:
            symbol = 'p';
            break;
        case PieceType::QUEEN:
            symbol = 'q';
            break;
        case PieceType::KING:
            symbol = 'k';
            break;
        case PieceType::ROOK:
            symbol = 'r';
            break;
        case PieceType::KNIGHT:
            symbol = 'n';
            break;
        case PieceType::BISHOP:
            symbol = 'b';
            break;
        default:
            break;
    }

    if (color == Color::WHITE) {
        symbol = std::toupper(symbol);
    }

    return symbol;
}

} // namespace chess
