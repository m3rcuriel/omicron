#include "particle_filter.h"
#include <cassert>
#include "util.h"

namespace chess {

namespace agent {

namespace {

bool handle_board_piece_no_obs(Board &board, Piece board_piece,
                               Position obs_pos, Position origin) {
  while (true) {
    Position new_pos;
    if (board_piece.type == PieceType::BISHOP) {
      int random_idx = random_int(32);
      random_idx *= 2;
      if ((obs_pos.rank + obs_pos.file % 2) == 0) {
        new_pos = {random_idx / 8, random_idx % 8};
      } else {
        random_idx++;
        new_pos = {random_idx / 8, random_idx % 8};
      }
    } else {
      new_pos = {random_int(8), random_int(8)};
    }

    if (board.get_piece(new_pos.rank, new_pos.file) == Piece::EMPTY &&
        !(new_pos.rank >= origin.rank && new_pos.rank < origin.rank + 3 &&
          new_pos.file >= origin.file && new_pos.file < origin.file + 3 &&
          new_pos.rank * 8 + new_pos.file <= obs_pos.rank * 8 + obs_pos.file)) {
      board.set_piece(new_pos.rank, new_pos.file, board_piece);
      board.set_piece(obs_pos.rank, obs_pos.file, Piece::EMPTY);
      return true;
    }
  }
}

bool handle_obs_piece_no_board(Board &board, Piece obs_piece, Position obs_pos,
                               Position origin) {
  ::std::vector<Position> temp_res = board.find_all_piece(obs_piece);
  ::std::vector<Position> res;
  for (Position pos : temp_res) {
    if (pos.rank >= origin.rank && pos.rank < origin.rank + 3 &&
        pos.file >= origin.file && pos.file < origin.file + 3 &&
        pos.rank * 8 + pos.file <= obs_pos.rank * 8 + obs_pos.file) {
      if (board.get_piece(pos.rank, pos.file).type == PieceType::PAWN) {
        //          res.push_back(pos);
      }
      // this piece is invalid
    } else {
      res.push_back(pos);
    }
  }

  Position selected;
  switch (obs_piece.type) {
    case PieceType::BISHOP: {
      if (res.size() == 0) {
        ::std::cout << "Found no bishops" << std::endl;
        return false;
      } else if (res.size() == 1) {
        if ((res.at(0).rank + res.at(0).file) % 2 !=
            (obs_pos.rank + obs_pos.file) % 2) {
          ::std::cout << "Looking for single bishop no match" << std::endl;
          ::std::cout << "obs_pos: " << obs_pos << std::endl;
          ::std::cout << "res 0: " << res.at(0) << std::endl;
          ::std::cout << "temp_res size: " << temp_res.size() << std::endl;
          board.debug_print(std::cout);
          return false;
        }
        selected = res.at(0);
      } else {
        bool match = false;
        for (auto pos : res) {
          if ((pos.rank + pos.file) % 2 == (obs_pos.rank + obs_pos.file) % 2) {
            selected = pos;
            match = true;
            break;
          }
        }

        if (!match) {
          std::cout << "BISHOP NO MATCH" << std::endl;
          return false;
        }
      }
      break;
    }
    case PieceType::PAWN: {
#if 0
      if (res.size() == 0) {
        return false;
      }
      for (Position pos : res) {
        if (pos.file == obs_pos.file) {
          selected = pos;
          break;
        }
      }

      if (board.get_piece(selected.rank, selected.file) != Piece::EMPTY) {
        break;
      }
#endif
    }
    case PieceType::KNIGHT:
    case PieceType::ROOK:
    case PieceType::QUEEN:
    case PieceType::KING: {
      if (res.size() == 0) {
        ::std::cout << "Couldn't find piece " << obs_piece << std::endl;
        auto valid_pieces =
            board.find_all_valid_color(obs_piece.color, obs_pos);
        auto chosen = random_choice(valid_pieces);
        auto chosen_piece = board.get_piece(chosen.rank, chosen.file);
        chosen_piece.type = obs_piece.type;
        board.set_piece(chosen.rank, chosen.file, chosen_piece);
        selected = chosen;
        break;
      }

      selected = random_choice(res);
      break;
    }
  }

  assert(selected != Position::NONE);
  board.set_piece(obs_pos.rank, obs_pos.file, obs_piece);
  board.set_piece(selected.rank, selected.file, Piece::EMPTY);

  return true;
}
}  // namespace

Board StateDistribution::sample() const { return random_choice(particles); }

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

std::tuple<double, std::vector<std::tuple<int, Move, StateDistribution>>>
StateDistribution::update(Move move, Color our_color) const {
  int num_wins = 0;
  int initial_size = particles.size();

  std::vector<std::tuple<int, Move, StateDistribution>> result;
  std::map<Move, int> indices;
  for (Board b : particles) {
    assert(b.get_piece(move.from.rank, move.from.file).color == our_color);
    MoveResult move_result = b.apply_move(move);

    if (move_result.capture.piece.type == PieceType::KING) {
      num_wins += 20;
    }

    if (indices.find(move_result.move) == indices.end()) {
      indices[move_result.move] = result.size();
      result.push_back(
          std::make_tuple(1, move_result.move, StateDistribution({b})));
    } else {
      std::get<2>(result[indices[move_result.move]]).particles.push_back(b);
    }
  }

  for (auto &it : result) {
    std::get<2>(it).CheckValid(our_color);
    std::vector<Board> &it_particles = std::get<2>(it).particles;
    while (it_particles.size() < particles.size()) {
      Board b = random_choice(it_particles);
      it_particles.push_back(b);
    }
  }

  return std::make_tuple(((double)num_wins) / initial_size, result);
}

double color_value(Color color, Color ours) {
  if (color == ours) {
    return 1;
  } else if (color == opponent(ours)) {
    return -1.0;
  } else {
    return 0;
  }
}

int mirrored_rank(Color color, int rank) {
  if (color == Color::WHITE) {
    return rank;
  } else {
    return 7 - rank;
  }
}

double StateDistribution::heuristic_value(Color color) const {
  double piece_values = 0;
  for (int c = 0; c < 10; c++) {
    const Board &b = random_choice(particles);
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        Piece piece = b.get_piece(i, j);
        piece_values +=
            (piece_value(piece.type) + mirrored_rank(piece.color, i)) *
            color_value(piece.color, color);
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
    Capture capture = b.do_random_move(opponent_color).capture;
    if (indices.find(capture) == indices.end()) {
      indices[capture] = result.size();
      result.push_back(std::make_tuple(1, capture, StateDistribution({b})));
    } else {
      std::get<2>(result[indices[capture]]).particles.push_back(b);
    }
  }

  for (auto &it : result) {
    std::vector<Board> &it_particles = std::get<2>(it).particles;
    while (it_particles.size() < particles.size()) {
      Board b = random_choice(it_particles);
      it_particles.push_back(b);
    }
  }
  return result;
}

std::vector<Move> StateDistribution::get_available_actions(Color color) const {
  return particles[0].generate_moves(color);
}

static bool is_valid(const Board &b, Observation obs) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (b.get_piece(obs.origin.rank + i, obs.origin.file + j) !=
          obs.obs[i][j]) {
        return false;
      }
    }
  }
  return true;
}

void StateDistribution::observe(Observation obs, Color our_color) {
  std::vector<Board> result_particles;
  while (result_particles.size() < particles.size()) {
    //    ::std::cout << result_particles.size() << std::endl;
    Board b = random_choice(particles);
    if (coerce_board(b, obs, our_color)) {
      if (is_valid(b, obs)) {
        result_particles.push_back(b);
      } else {
        b.debug_print(std::cout);
        for (int i = 2; i >= 0; --i) {
          for (int j = 0; j < 3; ++j) {
            std::cout << obs.obs[i][j] << ", ";
          }
          std::cout << ::std::endl;
        }
        ::std::cout << "is not valid" << std::endl;
      }
    } else {
      ::std::cout << "could not coerce" << std::endl;
    }
  }
  std::swap(particles, result_particles);
}

void StateDistribution::handle_move_result(Move taken_move, Color our_color,
                                           bool capture,
                                           Position captured_position) {
  CheckValid(our_color);
  std::vector<Board> result_particles;
  while (result_particles.size() < particles.size()) {
    Board b = random_choice(particles);

    if (taken_move.from == taken_move.to) {
      return;
    }

    if (capture) {
      if (b.get_piece(captured_position.rank, captured_position.file).color ==
          Color::EMPTY) {
        auto opponent_pieces =
            b.find_all_valid_color(opponent(our_color), captured_position);
        Position chosen = random_choice(opponent_pieces);
        Piece chosen_piece = b.get_piece(chosen.rank, chosen.file);
        b.set_piece(chosen.rank, chosen.file, Piece::EMPTY);
        b.set_piece(captured_position.rank, captured_position.file,
                    chosen_piece);
      } else if (b.get_piece(captured_position.rank, captured_position.file)
                     .type == PieceType::KING) {
        continue;
      }
    } else {
      handle_board_piece_no_obs(
          b, b.get_piece(taken_move.to.rank, taken_move.to.file), {0, 0},
          {taken_move.to.rank, taken_move.to.rank});
    }

    MoveResult result = b.move_piece(taken_move.from, taken_move.to);
    if (result.move.to == taken_move.to) {
      result_particles.push_back(b);
    }
  }
  std::swap(particles, result_particles);
  CheckValid(our_color);
}

bool StateDistribution::coerce_board(Board &board, Observation obs,
                                     Color color) {
  for (int rank_idx = 0; rank_idx < 3; ++rank_idx) {
    for (int file_idx = 0; file_idx < 3; ++file_idx) {
      Piece obs_piece = obs.obs[rank_idx][file_idx];
      Position obs_pos = {rank_idx + obs.origin.rank,
                          file_idx + obs.origin.file};
      Piece board_piece = board.get_piece(obs_pos.rank, obs_pos.file);
      if (obs_piece.color != color) {
        // only handle enemy pieces
        if (obs_piece != board_piece) {
          // only care if we have a mismatch
          if (obs_piece != Piece::EMPTY) {
            if (board_piece == Piece::EMPTY) {
              // the board is missing a piece
              std::cout << "obs_piece_no_board" << std::endl;
              bool res = handle_obs_piece_no_board(board, obs_piece, obs_pos,
                                                   obs.origin);
              if (!res) {
                std::cout << "obs_piece_no_board" << std::endl;
                return false;
              }
            } else {
              // the observation and board mismatch
              bool res = handle_board_piece_no_obs(board, board_piece, obs_pos,
                                                   obs.origin);
              std::cout << "temp handle_board_piece_no_obs" << std::endl;
              if (!res) {
                std::cout << "temp handle_board_piece_no_obs" << std::endl;
              }
              res &= handle_obs_piece_no_board(board, obs_piece, obs_pos,
                                               obs.origin);
              std::cout << "temp handle_obs_piece_no_board" << std::endl;
              if (!res) {
                std::cout << "temp handle_obs_piece_no_board" << std::endl;
                return false;
              }
            }
          } else {
            // the observation is empty but we expected something
            std::cout << "handle_board_piece_no_obs" << std::endl;
            std::cout << board_piece << " " << obs_pos << std::endl;
            bool res = handle_board_piece_no_obs(board, board_piece, obs_pos,
                                                 obs.origin);
            if (!res) {
              std::cout << "handle_board_piece_no_obs" << std::endl;
              return false;
            }
          }
        }
      } else {
        assert(obs_piece == board_piece);
      }
    }
  }

  return true;
}

StateDistribution StateDistribution::subsample(size_t num) {
  std::vector<Board> result;
  for (size_t i = 0; i < num; i++) {
    result.push_back(random_choice(particles));
  }
  return StateDistribution(std::move(result));
}

void StateDistribution::reinitialize(Board board) {
  std::vector<Board> new_particles(kNumParticles, board);
  std::swap(new_particles, particles);
}

void StateDistribution::entropy(std::array<std::array<double, 8>, 8> &out,
                                Color our_color) const {
  std::array<std::array<std::array<int, 8>, 8>, 7> piece_counts;

  for (auto &b : piece_counts) {
    for (auto &r : b) {
      r.fill(0);
    }
  }

  for (const Board &p : particles) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if (p.get_piece(i, j).color != our_color) {
          int key = static_cast<int>(p.get_piece(i, j).type);
          piece_counts[key][i][j]++;
        }
      }
    }
  }

  for (auto &count : piece_counts) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if (count[i][j] != 0) {
          double prob = static_cast<double>(count[i][j]) / particles.size();
          out[i][j] -= prob * std::log2(prob);
        }
      }
    }
  }
}

double StateDistribution::square_entropy(Position position) const {
  std::map<Piece, int> piece_counts;
  for (const Board &p : particles) {
    Piece piece = p.get_piece(position.rank, position.file);
    auto it = piece_counts.find(piece);
    if (it == piece_counts.end()) {
      piece_counts.insert({piece, 1});
    } else {
      it->second++;
    }
  }

  double entropy = 0.0;
  for (auto it : piece_counts) {
    if (it.second > 0) {
      double prob = static_cast<double>(it.second) / particles.size();
      entropy -= prob * std::log2(prob);
    }
  }
  return entropy;
}

void StateDistribution::handle_opponent_move_result(bool captured_piece,
                                                    Position capture,
                                                    Color opponent_color) {
  CheckValid(opponent(opponent_color));
  std::vector<Board> new_particles;
  while (new_particles.size() < particles.size()) {
    Board b = random_choice(particles);
    if (!captured_piece) {
      MoveResult result = b.do_random_move(opponent_color);
      if (result.capture == Capture::NONE) {
        new_particles.push_back(b);
      }
    } else {
      auto opponent_pieces = b.find_all_valid_color(opponent_color, capture);
      Position chosen = random_choice(opponent_pieces);
      Piece chosen_piece = b.get_piece(chosen.rank, chosen.file);
      b.set_piece(chosen.rank, chosen.file, Piece::EMPTY);
      b.set_piece(capture.rank, capture.file, chosen_piece);
      new_particles.push_back(b);
    }
  }
  std::swap(particles, new_particles);
  CheckValid(opponent(opponent_color));
}

void StateDistribution::CheckValid(Color color) const {
  size_t z = 0;
  for (Board b : particles) {
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++j) {
        if (b.get_piece(i, j).color == color) {
          if (b.get_piece(i, j) != particles.at(0).get_piece(i, j)) {
            assert(false);
          }
        }
      }
    }
    z++;
  }
}

}  // namespace agent
}  // namespace chess

