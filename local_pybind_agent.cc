#include "chess_agent.h"

namespace py = pybind11;

using chess::agent::ChessAgent;

PYBIND11_MODULE(chess_agent, m) {
  py::class_<ChessAgent>(m, "ChessAgent")
      .def(py::init<>())
      .def("handle_game_start", &ChessAgent::handle_game_start)
      .def("handle_opponent_move_result",
           &ChessAgent::handle_opponent_move_result)
      .def("choose_sense", &ChessAgent::choose_sense)
      .def("handle_sense_result", &ChessAgent::handle_sense_result)
      .def("choose_move", &ChessAgent::choose_move)
      .def("handle_move_result", &ChessAgent::handle_move_result)
      .def("handle_game_end", &ChessAgent::handle_game_end);
}
