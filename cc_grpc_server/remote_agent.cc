#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "chess.h"
#include "chess_agent.h"
#include "server_agent/agent.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using agent::ChooseMoveReply;
using agent::ChooseMoveRequest;
using agent::ChooseSenseReply;
using agent::ChooseSenseRequest;
using agent::HandleGameEndRequest;
using agent::HandleGameStartRequest;
using agent::HandleMoveResultRequest;
using agent::HandleOpponentMoveRequest;
using agent::HandleSenseResultRequest;
using agent::RemoteAgent;

namespace {
chess::Color ProtobufColorToChess(agent::Color color) {
  if (color == agent::Color::WHITE) {
    return chess::Color::WHITE;
  }
  return chess::Color::BLACK;
}

chess::Position ProtobufPositionToChess(agent::Position position) {
  return chess::Position(position.rank(), position.file());
}

chess::Move ProtobufMoveToChess(agent::Move move) {
  return {ProtobufPositionToChess(move.from_square()),
          ProtobufPositionToChess(move.to_square())};
}

void ChessPositionToProtobuf(chess::Position position,
                             agent::Position *agent_position) {
  agent_position->set_rank(position.rank);
  agent_position->set_file(position.file);
}

chess::Piece ProtobufPieceToChess(agent::Piece piece) {
  chess::Piece chess_piece;
  switch (piece.piece_type()) {
    case agent::PieceType::PAWN:
      chess_piece.type = chess::PieceType::PAWN;
      break;
    case agent::PieceType::BISHOP:
      chess_piece.type = chess::PieceType::BISHOP;
      break;
    case agent::PieceType::KNIGHT:
      chess_piece.type = chess::PieceType::KNIGHT;
      break;
    case agent::PieceType::ROOK:
      chess_piece.type = chess::PieceType::ROOK;
      break;
    case agent::PieceType::KING:
      chess_piece.type = chess::PieceType::KING;
      break;
    case agent::PieceType::QUEEN:
      chess_piece.type = chess::PieceType::QUEEN;
      break;
    default:
      chess_piece.type = chess::PieceType::EMPTY;
      chess_piece.color = chess::Color::EMPTY;
      return chess_piece;
  }

  chess_piece.color = ProtobufColorToChess(piece.color());

  return chess_piece;
}

void ChessMoveToProtobuf(chess::Move move, agent::Move *agent_move) {
  ChessPositionToProtobuf(move.to, agent_move->mutable_to_square());
  ChessPositionToProtobuf(move.from, agent_move->mutable_from_square());
}

}  // namespace

class ChessAgentImpl final : public RemoteAgent::Service {
 public:
  explicit ChessAgentImpl() {}

  Status HandleGameStart(ServerContext *context,
                         const HandleGameStartRequest *request,
                         google::protobuf::Empty *reply) override {
    ::std::cout << "Handling game start" << std::endl;
    agent_.reset(new chess::agent::ChessAgent());
    agent_->handle_game_start(ProtobufColorToChess(request->color()));

    return Status::OK;
  }

  Status HandleOpponentMove(ServerContext *context,
                            const HandleOpponentMoveRequest *request,
                            google::protobuf::Empty *reply) override {
    ::std::cout << "Handling opponent move" << std::endl;
    chess::Position captured_position =
        request->has_captured_square()
            ? ProtobufPositionToChess(request->captured_square())
            : chess::Position(0, 0);
    agent_->handle_opponent_move_result(request->has_captured_square(),
                                        captured_position);

    return Status::OK;
  }

  Status ChooseSense(ServerContext *context, const ChooseSenseRequest *request,
                     ChooseSenseReply *reply) override {
    ::std::cout << "Handling choose sense" << std::endl;
    ::std::vector<chess::Position> possible_sense(
        request->possible_sense_size());
    ::std::vector<chess::Move> possible_moves(request->possible_moves_size());

    ::std::transform(request->possible_sense().begin(),
                     request->possible_sense().end(), possible_sense.begin(),
                     [](const agent::Position &pos) {
                       return ProtobufPositionToChess(pos);
                     });

    ::std::transform(
        request->possible_moves().begin(), request->possible_moves().end(),
        possible_moves.begin(),
        [](const agent::Move &move) { return ProtobufMoveToChess(move); });

    auto sense_location = agent_->choose_sense(possible_sense, possible_moves,
                                               request->seconds_left());

    std::cout << "Sensing: " << sense_location << std::endl;

    ChessPositionToProtobuf(sense_location, reply->mutable_sense_location());

    return Status::OK;
  }

  Status HandleSenseResult(ServerContext *context,
                           const HandleSenseResultRequest *request,
                           google::protobuf::Empty *reply) override {
    ::std::cout << "Handling sense result" << std::endl;

    ::std::vector<agent::SenseResult> raw_obs(request->result().begin(),
                                              request->result().end());
    ::std::cout << std::endl;
    std::sort(
        raw_obs.begin(), raw_obs.end(),
        [](const agent::SenseResult &first, const agent::SenseResult &second) {
          if (first.square().rank() == second.square().rank()) {
            return first.square().file() < second.square().file();
          }
          return first.square().rank() < second.square().rank();
        });

    chess::Observation obs;

    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        if (raw_obs[j * 3 + i].has_piece()) {
          obs.obs[i][j] =
              ProtobufPieceToChess(raw_obs[i * 3 + j].piece());  // probably
        } else {
          obs.obs[i][j] = chess::Piece::EMPTY;
        }
      }
    }

    obs.origin = ProtobufPositionToChess(raw_obs[0].square());

    agent_->handle_sense_result(obs);

    std::cout << "Finished handle sense result" << std::endl;

    return Status::OK;
  }

  Status ChooseMove(ServerContext *context, const ChooseMoveRequest *request,
                    ChooseMoveReply *reply) {
    ::std::cout << "Handling choose move" << std::endl;
    chess::Move move = agent_->choose_move(request->seconds_left());

    ChessMoveToProtobuf(move, reply->mutable_move());

    reply->mutable_move();

    return Status::OK;
  }

  Status HandleMoveResult(ServerContext *context,
                          const HandleMoveResultRequest *request,
                          google::protobuf::Empty *reply) {
    if (!request->has_taken_move()) {
      chess::Move requested_move =
          ProtobufMoveToChess(request->requested_move());
      requested_move.to = requested_move.from;
      agent_->handle_move_result(requested_move, false, requested_move.to);
    } else {
      chess::Move taken_move = ProtobufMoveToChess(request->taken_move());
      agent_->handle_move_result(
          taken_move, request->has_captured_position(),
          ProtobufPositionToChess(request->captured_position()));
    }
    return Status::OK;
  }

  Status HandleGameEnd(ServerContext *context,
                       const HandleGameEndRequest *request,
                       google::protobuf::Empty *reply) {
    ::std::cout << "Handling game end" << std::endl;
    agent_->handle_game_end(ProtobufColorToChess(request->winner_color()),
                            request->win_reason());
    agent_.reset(new chess::agent::ChessAgent());
    return Status::OK;
  }

 private:
  ::std::unique_ptr<chess::agent::ChessAgent> agent_{
      new chess::agent::ChessAgent()};
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  ChessAgentImpl service;

  ServerBuilder builder;

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cout << "Listening on " << server_address << std::endl;

  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
