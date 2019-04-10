#include <pybind11/pybind11.h>
#include <random>

#include "chess.h"

namespace py = pybind11;

class RandomAgent {
 public:
  RandomAgent() {}

  void handle_game_start(py::object color, py::object board) {}
  void handle_opponent_move_result(py::object captured_piece,
                                   py::object captured_square) {}
  py::object choose_sense(py::list possible_sense, py::list possible_moves,
                          double seconds_left) {
    return possible_sense[0];
  }
  void handle_sense_result(py::object sense_result) {}
  py::object choose_move(py::list possible_moves, double seconds_left) {
    std::uniform_int_distribution<int> move_generator(0, possible_moves.size());
    return possible_moves[move_generator(generator)];
  }

  void handle_move_result(py::object requested_move, py::object taken_move,
                          py::object reason, py::object captured_piece,
                          py::object captured_square) {}
  void handle_game_end(py::object winner_color, py::object reason) {}

 private:
  std::default_random_engine generator;
};

PYBIND11_MODULE(chess_agent, m) {
  py::class_<RandomAgent>(m, "RandomAgent")
      .def(py::init<>())
      .def("handle_game_start", &RandomAgent::handle_game_start)
      .def("handle_opponent_move_result",
           &RandomAgent::handle_opponent_move_result)
      .def("choose_sense", &RandomAgent::choose_sense)
      .def("handle_sense_result", &RandomAgent::handle_sense_result)
      .def("choose_move", &RandomAgent::choose_move)
      .def("handle_move_result", &RandomAgent::handle_move_result)
      .def("handle_game_end", &RandomAgent::handle_game_end);
}
