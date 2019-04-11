#include <gtest/gtest.h>
#include "uct.h"

namespace chess {

namespace agent {

namespace test {

TEST(Uct, MakesTrivialMove) {
    Board starting_board  = Board::initial_board();

    for (int i = 0; i < 8; i++) {
        starting_board.set_piece(1, i, Piece::EMPTY);
        starting_board.set_piece(6, i, Piece::EMPTY);
    }

    OurUctNode root(starting_board, Color::WHITE);

    for (int i = 0; i < 1000; i++) {
        root.simulate(15);
    }
    root.print_moves();
    /*
    for (const UcbEntry& e : root.entries) {
        std::cout << "Move (" << (int) e.move.from.rank << ", " << (int) e.move.from.file << ") -> (" << (int) e.move.to.rank << ", " << (int) e.move.to.file << ") with value " << e.get_value(root.children[e.child_index].num_tries) << " and UCB " << e.get_ucb(root.num_tries, root.children[e.child_index].num_tries) << std::endl;
    }
    */
}

} // namespace test

} // namespace agent

} // namespace chess
