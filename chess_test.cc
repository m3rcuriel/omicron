#include <utility>
#include <gtest/gtest.h>
#include "chess.h"

namespace chess {

namespace test {

TEST(Chess, InitialBoard) {
    chess::Board board = chess::Board::initial_board();

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(board.get_piece(0, i).color, chess::Color::WHITE);
        EXPECT_EQ(board.get_piece(7, i).color, chess::Color::BLACK);
        EXPECT_EQ(board.get_piece(0, i).type, board.get_piece(7, i).type);
        EXPECT_EQ(board.get_piece(1, i).type, chess::PieceType::PAWN);
        EXPECT_EQ(board.get_piece(6, i).type, chess::PieceType::PAWN);
    }

    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            EXPECT_EQ(board.get_piece(i, j).type, chess::PieceType::EMPTY);
            EXPECT_EQ(board.get_piece(i, j).color, chess::Color::EMPTY);
        }
    }
}

void expect_moves(Board board, Color turn, std::vector<std::pair<Position, Position>> expected_moves) {
    auto moves = board.generate_moves(chess::Color::WHITE);
    EXPECT_EQ(expected_moves.size(), moves.size());
#if 0
    for (auto m : moves) {
        std::cout << (int) m.from.rank << ", " << (int) m.from.file << " -> " << (int) m.to.rank << ", " << (int) m.to.file << std::endl;
    }
#endif
    for (auto e : expected_moves) {
        EXPECT_TRUE(
                std::find_if(
                    moves.begin(),
                    moves.end(),
                    [=](Move m) {
                        return m.from == e.first && m.to == e.second;
                    }) != moves.end()) << "Did not find expected move " << (int) e.first.rank << ", " << (int) e.first.file << " -> " << (int) e.second.rank << ", " << (int) e.second.file;
    }
}

TEST(CHESS, BasicPawnMoves) {
    chess::Board board;
    board.set_piece(1, 0, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(3, 1, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{1, 0}, {2, 0}},
            {{1, 0}, {3, 0}},
            {{3, 1}, {4, 1}}
        }
    );
}

TEST(CHESS, PawnCapture) {
    chess::Board board;
    // Capture in each direction
    board.set_piece(3, 1, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(4, 0, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(4, 2, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    // Cannot move forwards if blocked
    board.set_piece(1, 3, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(2, 3, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    // Cannot move doubly if blocked
    board.set_piece(1, 5, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(3, 5, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{3, 1}, {4, 1}},
            {{3, 1}, {4, 0}},
            {{3, 1}, {4, 2}},
            {{1, 5}, {2, 5}},
        }
    );
}

TEST(CHESS, QueenMoves) {
    chess::Board board;
    /*   0 1 2 3 4 5 6 7
     * 7|
     * 6|
     * 5|
     * 4|.   p
     * 3|  . . p
     * 2|. . Q . . p
     * 1|  . . p
     * 0|.   .
     */
    board.set_piece(2, 2, chess::Piece{chess::Color::WHITE, chess::PieceType::QUEEN});

    board.set_piece(1, 3, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(4, 2, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(3, 3, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(2, 5, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{2, 2}, {4, 0}},
            {{2, 2}, {4, 2}},
            {{2, 2}, {3, 1}},
            {{2, 2}, {3, 2}},
            {{2, 2}, {3, 3}},
            {{2, 2}, {2, 0}},
            {{2, 2}, {2, 1}},
            {{2, 2}, {2, 3}},
            {{2, 2}, {2, 4}},
            {{2, 2}, {2, 5}},
            {{2, 2}, {1, 1}},
            {{2, 2}, {1, 2}},
            {{2, 2}, {1, 3}},
            {{2, 2}, {0, 0}},
            {{2, 2}, {0, 2}},
        }
    );
}

TEST(CHESS, KingMoves) {
    chess::Board board;
    /*   0 1 2 3 4 5 6 7
     * 7|. .
     * 6|K .
     * 5|. .
     * 4|
     * 3|  . . p
     * 2|  . K .
     * 1|  . . p
     * 0|
     */
    board.set_piece(2, 2, chess::Piece{chess::Color::WHITE, chess::PieceType::KING});
    board.set_piece(6, 0, chess::Piece{chess::Color::WHITE, chess::PieceType::KING});

    board.set_piece(1, 3, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(3, 3, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{2, 2}, {3, 1}},
            {{2, 2}, {3, 2}},
            {{2, 2}, {3, 3}},
            {{2, 2}, {2, 3}},
            {{2, 2}, {1, 3}},
            {{2, 2}, {1, 2}},
            {{2, 2}, {1, 1}},
            {{2, 2}, {2, 1}},
            {{6, 0}, {7, 0}},
            {{6, 0}, {7, 1}},
            {{6, 0}, {6, 1}},
            {{6, 0}, {5, 1}},
            {{6, 0}, {5, 0}},
        }
    );
}

TEST(CHESS, BishopMoves) {
    chess::Board board;
    /*   0 1 2 3 4 5 6 7
     * 7|
     * 6|
     * 5|          p
     * 4|.       .
     * 3|  .   .
     * 2|    B
     * 1|  .   .
     * 0|.       .
     */
    board.set_piece(2, 2, chess::Piece{chess::Color::WHITE, chess::PieceType::BISHOP});

    board.set_piece(5, 5, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{2, 2}, {0, 0}},
            {{2, 2}, {1, 1}},
            {{2, 2}, {3, 3}},
            {{2, 2}, {4, 4}},
            {{2, 2}, {5, 5}},
            {{2, 2}, {4, 0}},
            {{2, 2}, {3, 1}},
            {{2, 2}, {1, 3}},
            {{2, 2}, {0, 4}},
        }
    );
}

TEST(CHESS, KnightMoves) {
    chess::Board board;
    /*   0 1 2 3 4 5 6 7
     * 7|
     * 6|
     * 5|
     * 4|.   p
     * 3|      .
     * 2|  N
     * 1|      .
     * 0|.   .
     */
    board.set_piece(2, 1, chess::Piece{chess::Color::WHITE, chess::PieceType::KNIGHT});

    board.set_piece(4, 2, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{2, 1}, {4, 0}},
            {{2, 1}, {4, 2}},
            {{2, 1}, {3, 3}},
            {{2, 1}, {1, 3}},
            {{2, 1}, {0, 0}},
            {{2, 1}, {0, 2}},
        }
    );
}

TEST(CHESS, Castling) {
    Board board = Board::initial_board();

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

    {
        // No castling allowed with default board
        auto king_moves = board.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [](Move m) {
                        return m.piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 0);
    }
    {
        // Queenside castling
        Board board_copy = board;
        board_copy.set_piece(0, 1, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 2, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 3, Piece{Color::EMPTY, PieceType::EMPTY});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [](Move m) {
                        return m.piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 2);
        auto castle_move = Move{Piece{Color::WHITE, PieceType::KING}, {0, 4}, {0, 2}};
        board_copy.apply_move(castle_move);
        EXPECT_EQ(board_copy.get_piece(0, 2).type, PieceType::KING);
        EXPECT_EQ(board_copy.get_piece(0, 3).type, PieceType::ROOK);
        EXPECT_EQ(board_copy.get_piece(0, 0).type, PieceType::EMPTY);
    }
    {
        // No queenside castling if rook has been moved
        Board board_copy = board;
        board_copy.set_piece(0, 1, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 2, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 3, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.apply_move(Move{Piece{Color::WHITE, PieceType::ROOK}, {0, 0}, {0, 1}});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [](Move m) {
                        return m.piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 1);
    }
    {
        // No queenside castling if king has been moved
        Board board_copy = board;
        board_copy.set_piece(0, 1, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 2, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 3, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.apply_move(Move{Piece{Color::WHITE, PieceType::KING}, {0, 4}, {0, 3}});
        board_copy.apply_move(Move{Piece{Color::WHITE, PieceType::KING}, {0, 3}, {0, 4}});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [](Move m) {
                        return m.piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 1);
    }
    {
        // Kingside castling
        Board board_copy = board;
        board_copy.set_piece(0, 5, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 6, Piece{Color::EMPTY, PieceType::EMPTY});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [](Move m) {
                        return m.piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 2);
        auto castle_move = Move{Piece{Color::WHITE, PieceType::KING}, {0, 4}, {0, 6}};
        board_copy.apply_move(castle_move);
        EXPECT_EQ(board_copy.get_piece(0, 6).type, PieceType::KING);
        EXPECT_EQ(board_copy.get_piece(0, 5).type, PieceType::ROOK);
        EXPECT_EQ(board_copy.get_piece(0, 7).type, PieceType::EMPTY);
    }
    {
        // No kingside castling if moved
        Board board_copy = board;
        board_copy.set_piece(0, 5, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.set_piece(0, 6, Piece{Color::EMPTY, PieceType::EMPTY});
        board_copy.apply_move(Move{Piece{Color::WHITE, PieceType::ROOK}, {0, 7}, {0, 6}});
        board_copy.apply_move(Move{Piece{Color::WHITE, PieceType::ROOK}, {0, 6}, {0, 7}});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [](Move m) {
                        return m.piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 1);
    }
}

TEST(Chess, EnPassant) {
    chess::Board board = chess::Board::initial_board();

    board.set_piece(1, 0, Piece{Color::EMPTY, PieceType::EMPTY});
    board.set_piece(4, 0, Piece{Color::WHITE, PieceType::PAWN});
    board.apply_move(Move{Piece{Color::BLACK, PieceType::PAWN}, {6, 1}, {4, 1}});

    auto ep_pawn_moves = board.generate_moves(Color::WHITE);
    ep_pawn_moves.erase(std::remove_if(
                ep_pawn_moves.begin(), ep_pawn_moves.end(),
                [](Move m) {
                    return m.from != Position{4, 0} || m.to != Position{5, 1};
                }
            ), ep_pawn_moves.end());
    ASSERT_EQ(ep_pawn_moves.size(), 1);
    Piece cap = board.apply_move(ep_pawn_moves[0]);
    EXPECT_EQ(board.get_piece(5, 1).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(5, 1).type, PieceType::PAWN);
    EXPECT_EQ(board.get_piece(4, 0).color, Color::EMPTY);
    EXPECT_EQ(board.get_piece(4, 0).type, PieceType::EMPTY);
    EXPECT_EQ(cap.color, Color::BLACK);
    EXPECT_EQ(cap.type, PieceType::PAWN);
}

} // namespace test

} // namespace chess
