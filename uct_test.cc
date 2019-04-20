#include <gtest/gtest.h>
#include "uct.h"

namespace chess {

namespace agent {

namespace test {

TEST(Uct, MakesTrivialMove) {
    Board starting_board = Board::initial_board();

    OurUctNode root(starting_board, Color::WHITE);

    for (int i = 0; i < 1000; i++) {
        root.simulate(10);
    }
    root.print_moves();
}

} // namespace test

} // namespace agent

} // namespace chess
