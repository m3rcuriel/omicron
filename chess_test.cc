#include <utility>
#include <gtest/gtest.h>
#include "chess.h"

namespace chess {

namespace test {

TEST(Chess, ReprSizes) {
    EXPECT_EQ(sizeof(Piece), 1);
    EXPECT_LT(sizeof(Board), 80);
}

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

void expect_moves(
        Board board, Color turn,
        std::vector<std::pair<Position, Position>> expected_moves) {
    auto moves = board.generate_moves(turn);
    EXPECT_EQ(expected_moves.size(), moves.size());
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

TEST(Chess, BasicPawnMoves) {
    chess::Board board;
    board.set_piece(1, 0, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(3, 1, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{1, 0}, {2, 0}},
            {{1, 0}, {2, 1}},
            {{1, 0}, {3, 0}},
            {{3, 1}, {4, 1}},
            {{3, 1}, {4, 2}},
            {{3, 1}, {4, 0}}
        }
    );
}

TEST(Chess, PawnCapture) {
    chess::Board board;
    // Can't capture own piece
    board.set_piece(3, 1, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(4, 0, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(4, 2, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});

    // Cannot move forwards if blocked
    board.set_piece(1, 3, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(2, 3, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});

    // Cannot move doubly if blocked
    board.set_piece(1, 5, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});
    board.set_piece(3, 5, chess::Piece{chess::Color::WHITE, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{3, 1}, {4, 1}},

            {{4, 0}, {5, 0}},
            {{4, 0}, {5, 1}},

            {{4, 2}, {5, 1}},
            {{4, 2}, {5, 3}},
            {{4, 2}, {5, 2}},

            {{1, 3}, {2, 2}},
            {{1, 3}, {2, 4}},

            {{2, 3}, {3, 2}},
            {{2, 3}, {3, 3}},
            {{2, 3}, {3, 4}},

            {{1, 5}, {2, 4}},
            {{1, 5}, {2, 5}},
            {{1, 5}, {2, 6}},

            {{3, 5}, {4, 4}},
            {{3, 5}, {4, 5}},
            {{3, 5}, {4, 6}},
        }
    );
}

TEST(Chess, QueenMoves) {
    chess::Board board;
    /*   0 1 2 3 4 5 6 7
     * 7|    .         .
     * 6|    .       .
     * 5|    .     .
     * 4|.   p   .
     * 3|  . . p
     * 2|. . Q . . p . .
     * 1|  . . p
     * 0|.   .   .
     */
    board.set_piece(2, 2, chess::Piece{chess::Color::WHITE, chess::PieceType::QUEEN});

    board.set_piece(1, 3, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(4, 2, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(3, 3, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});
    board.set_piece(2, 5, chess::Piece{chess::Color::BLACK, chess::PieceType::PAWN});

    expect_moves(
        board, Color::WHITE,
        {
            {{2, 2}, {0, 0}},
            {{2, 2}, {0, 2}},
            {{2, 2}, {0, 4}},
            {{2, 2}, {1, 1}},
            {{2, 2}, {1, 2}},
            {{2, 2}, {1, 3}},
            {{2, 2}, {2, 0}},
            {{2, 2}, {2, 1}},
            {{2, 2}, {2, 3}},
            {{2, 2}, {2, 4}},
            {{2, 2}, {2, 5}},
            {{2, 2}, {2, 6}},
            {{2, 2}, {2, 7}},
            {{2, 2}, {3, 1}},
            {{2, 2}, {3, 2}},
            {{2, 2}, {3, 3}},
            {{2, 2}, {4, 0}},
            {{2, 2}, {4, 2}},
            {{2, 2}, {4, 4}},
            {{2, 2}, {5, 2}},
            {{2, 2}, {5, 5}},
            {{2, 2}, {6, 2}},
            {{2, 2}, {6, 6}},
            {{2, 2}, {7, 2}},
            {{2, 2}, {7, 7}},
        }
    );
}

TEST(Chess, KingMoves) {
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

TEST(Chess, BishopMoves) {
    chess::Board board;
    /*   0 1 2 3 4 5 6 7
     * 7|              .
     * 6|            .
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
            {{2, 2}, {6, 6}},
            {{2, 2}, {7, 7}},
            {{2, 2}, {4, 0}},
            {{2, 2}, {3, 1}},
            {{2, 2}, {1, 3}},
            {{2, 2}, {0, 4}},
        }
    );
}

TEST(Chess, KnightMoves) {
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

TEST(Chess, Castling) {
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
                    [&](Move m) {
                        Piece piece = board.get_piece(m.from.rank, m.from.file);
                        return piece.type != PieceType::KING;
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
                    [&](Move m) {
                        Piece piece = board.get_piece(m.from.rank, m.from.file);
                        return piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 2);
        auto castle_move = Move{{0, 4}, {0, 2}};
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
        board_copy.apply_move(Move{{0, 0}, {0, 1}});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [&](Move m) {
                        Piece piece = board.get_piece(m.from.rank, m.from.file);
                        return piece.type != PieceType::KING;
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
        board_copy.apply_move(Move{{0, 4}, {0, 3}});
        board_copy.apply_move(Move{{0, 3}, {0, 4}});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [&](Move m) {
                        Piece piece = board.get_piece(m.from.rank, m.from.file);
                        return piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_FALSE(board_copy.get_castle_kingside_white());
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
                    [&](Move m) {
                        Piece piece = board.get_piece(m.from.rank, m.from.file);
                        return piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 2);
        auto castle_move = Move{{0, 4}, {0, 6}};
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
        board_copy.apply_move(Move{{0, 7}, {0, 6}});
        board_copy.apply_move(Move{{0, 6}, {0, 7}});
        auto king_moves = board_copy.generate_moves(Color::WHITE);
        king_moves.erase(std::remove_if(
                    king_moves.begin(), king_moves.end(),
                    [&](Move m) {
                        Piece piece = board.get_piece(m.from.rank, m.from.file);
                        return piece.type != PieceType::KING;
                    }
                ), king_moves.end());
        EXPECT_EQ(king_moves.size(), 1);
    }
}

TEST(Chess, BasicMove) {
    chess::Board board;
    board.set_piece(3, 3, Piece{Color::WHITE, PieceType::QUEEN});
    board.apply_move(Move{{3, 3}, {1, 1}});
    EXPECT_EQ(board.get_piece(3, 3).color, Color::EMPTY);
    EXPECT_EQ(board.get_piece(3, 3).type, PieceType::EMPTY);
    EXPECT_EQ(board.get_piece(1, 1).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(1, 1).type, PieceType::QUEEN);
}

TEST(Chess, BlockedMoveOpponent) {
    chess::Board board;
    board.set_piece(4, 4, Piece{Color::WHITE, PieceType::QUEEN});
    board.set_piece(2, 2, Piece{Color::BLACK, PieceType::QUEEN});
    board.apply_move(Move{{4, 4}, {1, 1}});
    EXPECT_EQ(board.get_piece(1, 1).color, Color::EMPTY);
    EXPECT_EQ(board.get_piece(1, 1).type, PieceType::EMPTY);
    EXPECT_EQ(board.get_piece(2, 2).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(2, 2).type, PieceType::QUEEN);
}

TEST(Chess, BlockedMoveSelf) {
    chess::Board board;
    board.set_piece(4, 4, Piece{Color::WHITE, PieceType::QUEEN});
    board.set_piece(2, 2, Piece{Color::WHITE, PieceType::QUEEN});
    board.apply_move(Move{{4, 4}, {1, 1}});
    EXPECT_EQ(board.get_piece(2, 2).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(2, 2).type, PieceType::QUEEN);
    EXPECT_EQ(board.get_piece(3, 3).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(3, 3).type, PieceType::QUEEN);
}

TEST(Chess, PawnWasteMove) {
    chess::Board board = Board::initial_board();
    board.set_piece(3, 0, Piece{Color::WHITE, PieceType::PAWN});
    board.apply_move(Move{{1, 0}, {2, 1}});
    EXPECT_EQ(board.get_piece(1, 0).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(1, 0).type, PieceType::PAWN);
    EXPECT_EQ(board.get_piece(2, 1).color, Color::EMPTY);
    EXPECT_EQ(board.get_piece(2, 1).type, PieceType::EMPTY);
}

TEST(Chess, PawnBlocked) {
    chess::Board board = Board::initial_board();
    board.set_piece(3, 0, Piece{Color::BLACK, PieceType::PAWN});
    board.apply_move(Move{{1, 0}, {3, 0}});
    EXPECT_EQ(board.get_piece(1, 0).color, Color::EMPTY);
    EXPECT_EQ(board.get_piece(1, 0).type, PieceType::EMPTY);
    EXPECT_EQ(board.get_piece(2, 0).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(2, 0).type, PieceType::PAWN);
    EXPECT_EQ(board.get_piece(3, 0).color, Color::BLACK);
    EXPECT_EQ(board.get_piece(3, 0).type, PieceType::PAWN);
}

TEST(Chess, EnPassant) {
    chess::Board board = chess::Board::initial_board();

    board.set_piece(1, 0, Piece{Color::EMPTY, PieceType::EMPTY});
    board.set_piece(4, 0, Piece{Color::WHITE, PieceType::PAWN});
    board.apply_move(Move{{6, 1}, {4, 1}});

    auto ep_pawn_moves = board.generate_moves(Color::WHITE);
    ep_pawn_moves.erase(std::remove_if(
                ep_pawn_moves.begin(), ep_pawn_moves.end(),
                [](Move m) {
                    return m.from != Position{4, 0} || m.to != Position{5, 1};
                }
            ), ep_pawn_moves.end());
    ASSERT_EQ(ep_pawn_moves.size(), 1);
    Capture cap = board.apply_move(ep_pawn_moves[0]);
    EXPECT_EQ(board.get_piece(5, 1).color, Color::WHITE);
    EXPECT_EQ(board.get_piece(5, 1).type, PieceType::PAWN);
    EXPECT_EQ(board.get_piece(4, 0).color, Color::EMPTY);
    EXPECT_EQ(board.get_piece(4, 0).type, PieceType::EMPTY);
    EXPECT_EQ(cap.piece.color, Color::BLACK);
    EXPECT_EQ(cap.piece.type, PieceType::PAWN);
    EXPECT_EQ(cap.position, (Position{4, 1}));
}

TEST(Chess, BasicMoveBlack) {
    chess::Board board;
    board.set_piece(3, 3, Piece{Color::BLACK, PieceType::PAWN});
    EXPECT_EQ(board.generate_moves(Color::BLACK).size(), 3);
    EXPECT_EQ(board.generate_moves(Color::WHITE).size(), 0);
    board.apply_move(Move{{3, 3}, {2, 3}});
    expect_moves(
        board, Color::BLACK,
        {
            {{2, 3}, {1, 2}},
            {{2, 3}, {1, 3}},
            {{2, 3}, {1, 4}},
        }
    );
}

TEST(Chess, SimulateGame) {
    /*
     *
     *   R B Q K B N R
     * P P P P P P P P
     *     N
     *
     * p
     *
     *   p p p p p p p
     * r n b q k b n r
     */
    Board board = Board::initial_board();
    board.apply_move({{0, 1}, {2, 2}});
    board.apply_move({{6, 0}, {4, 0}});
    board.apply_move({{0, 0}, {0, 1}});
    auto king_moves = board.generate_moves(Color::WHITE);
    king_moves.erase(std::remove_if(
                king_moves.begin(), king_moves.end(),
                [&](Move m) {
                    Piece piece = board.get_piece(m.from.rank, m.from.file);
                    return piece.type != PieceType::KING;
                }
            ), king_moves.end());
    ASSERT_EQ(king_moves.size(), 0);
}

Move choose_random(std::vector<Move> moves) {
    return moves[rand() % moves.size()];
}

void expect_piece(Board& board, Piece piece) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Piece actual = board.get_piece(i, j);
            if (piece == actual) {
                board.set_piece(i, j, Piece::EMPTY);
                return;
            }
        }
    }
    assert(false);
}

void compare_tracking(Board board, std::vector<PieceType> tracked, Color color) {
    while (tracked.size()) {
        PieceType piece = tracked.back();
        tracked.pop_back();
        expect_piece(board, Piece{color, piece});
    }
}

TEST(Chess, TrackCaptures) {
    std::vector<PieceType> starting_pieces = {
        PieceType::PAWN,
        PieceType::PAWN,
        PieceType::PAWN,
        PieceType::PAWN,
        PieceType::PAWN,
        PieceType::PAWN,
        PieceType::PAWN,
        PieceType::PAWN,
        PieceType::ROOK,
        PieceType::KNIGHT,
        PieceType::BISHOP,
        PieceType::QUEEN,
        PieceType::KING,
        PieceType::BISHOP,
        PieceType::KNIGHT,
        PieceType::ROOK
    };

    for (int i = 0; i < 100; i++) {
        Board board = Board::initial_board();
        std::vector<PieceType> pieces_white = starting_pieces, pieces_black = starting_pieces;

        for (int i = 0; i < 35; i++) {
            Move white_move = choose_random(board.generate_moves(Color::WHITE));
            Capture captured_black = board.apply_move(white_move);

            if (captured_black != Capture::NONE) {
                auto captured_black_it = std::find(
                        pieces_black.begin(), pieces_black.end(), captured_black.piece.type);
                ASSERT_NE(captured_black_it, pieces_black.end());
                pieces_black.erase(captured_black_it);
            }

            Move black_move = choose_random(board.generate_moves(Color::BLACK));
            Capture captured_white = board.apply_move(black_move);

            if (captured_white != Capture::NONE) {
                auto captured_white_it = std::find(
                        pieces_white.begin(), pieces_white.end(), captured_white.piece.type);
                ASSERT_NE(captured_white_it, pieces_white.end());
                pieces_white.erase(captured_white_it);
            }

            compare_tracking(board, pieces_white, Color::WHITE);
            compare_tracking(board, pieces_black, Color::BLACK);
        }
    }
}

TEST(Chess, GameScenario) {
    /*
     *   0 1 2 3 4 5 6 7
     * 0
     * 1   P R Q B   P R
     * 2 P n N
     * 3
     * 4         p
     * 5 p p       n
     * 6   b p p k p p p
     * 7 r             r
     */
    Board board;
    board.set_piece(1, 1, Piece{Color::WHITE, PieceType::PAWN});
    board.set_piece(1, 2, Piece{Color::WHITE, PieceType::ROOK});
    board.set_piece(1, 3, Piece{Color::WHITE, PieceType::QUEEN});
    board.set_piece(1, 4, Piece{Color::WHITE, PieceType::BISHOP});
    board.set_piece(1, 6, Piece{Color::WHITE, PieceType::PAWN});
    board.set_piece(1, 7, Piece{Color::WHITE, PieceType::ROOK});
    board.set_piece(2, 0, Piece{Color::WHITE, PieceType::PAWN});
    board.set_piece(2, 1, Piece{Color::BLACK, PieceType::KNIGHT});
    board.set_piece(2, 2, Piece{Color::WHITE, PieceType::KNIGHT});
    board.set_piece(4, 4, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(5, 0, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(5, 1, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(5, 5, Piece{Color::BLACK, PieceType::KNIGHT});
    board.set_piece(6, 1, Piece{Color::BLACK, PieceType::BISHOP});
    board.set_piece(6, 2, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(6, 3, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(6, 4, Piece{Color::BLACK, PieceType::KING});
    board.set_piece(6, 5, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(6, 6, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(6, 7, Piece{Color::BLACK, PieceType::PAWN});
    board.set_piece(7, 0, Piece{Color::BLACK, PieceType::ROOK});
    board.set_piece(7, 7, Piece{Color::BLACK, PieceType::ROOK});
    board.debug_print(std::cout);
    board.apply_move({{5, 1}, {4, 1}});
    board.debug_print(std::cout);
}

} // namespace test

} // namespace chess
