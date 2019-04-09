#!/usr/bin/env python3

"""
File Name:      remote_agent.py
Authors:        Lee Mracek
Date:           April 9, 2019

Description:    Python file for an agent running on a remote server via gRPC
"""

import chess
from player import Player

import grpc
import agent_pb2
import agent_pb2_grpc

SERVER_IP = "localhost:50051"

def ChessColorToProtobuf(color):
    if color == chess.WHITE:
        return agent_pb2.WHITE
    elif color == chess.BLACK:
        return agent_pb2.BLACK
    raise ValueError("Please pass a valid chess color")

def ChessSquareToProtobufPosition(square):
    return agent_pb2.Position(
        rank = chess.square_rank(square),
        file = chess.square_file(square))

def ProtobufPositionToChessSquare(position):
    return chess.square(position.file, position.rank)

def ProtobufMoveToChess(move):
    to_position = ProtobufPositionToChessSquare(move.to_square)
    from_position = ProtobufPositionToChessSquare(move.from_square)
    promotion = None
    if move.has_promotion:
        promotion = move.promotion
    return chess.Move(from_position, to_position, promotion)

def ChessPieceToProtobuf(piece):
    type = ChessPieceTypeToProtobuf(piece.piece_type)
    return agent_pb2.Piece(piece_type = type,
            color = ChessColorToProtobuf(piece.color))

def ChessPieceTypeToProtobuf(piece_type):
    if piece_type == chess.PAWN:
        return agent_pb2.PAWN
    elif piece_type == chess.KNIGHT:
        return agent_pb2.KNIGHT
    elif piece_type == chess.BISHOP:
        return agent_pb2.BISHOP
    elif piece_type == chess.ROOK:
        return agent_pb2.ROOK
    elif piece_type == chess.QUEEN:
        return agent_pb2.QUEEN
    elif piece_type == chess.KING:
        return agent_pb2.KING
    raise ValueError("Please pass a valid chess piece")


def ChessMoveToProtobuf(move):
    to_position = ChessSquareToProtobufPosition(move.to_square)
    from_position = ChessSquareToProtobufPosition(move.from_square)
    if move.promotion is not None:
        promotion_piece = ChessPieceTypeToProtobuf(move.promotion)
        return agent_pb2.Move(
                to_square = to_position,
                from_square = from_position,
                has_promotion = True,
                promotion = promotion_piece)
    return agent_pb2.Move(
            to_square = to_position,
            from_square = from_position,
            has_promotion = False)


def ChessSenseResultToProtobuf(result):
    if result[1] is None:
        return agent_pb2.SenseResult(
            square = ChessSquareToProtobufPosition(result[0]))
    return agent_pb2.SenseResult(
        square = ChessSquareToProtobufPosition(result[0]),
        piece = ChessPieceToProtobuf(result[1]))


def MakeHandleGameStartRequest(color, board):
    proto_color = ChessColorToProtobuf(color)

    return agent_pb2.HandleGameStartRequest(
        color = proto_color)

def MakeHandleOpponentMoveRequest(captured_square):
    if captured_square is None:
        return agent_pb2.HandleOpponentMoveRequest()
    captured_square_proto = ChessSquareToProtobufPosition(captured_square)
    return agent_pb2.HandleOpponentMoveRequest(captured_square=captured_square_proto)

def MakeChooseSenseRequest(possible_sense, possible_moves, seconds_left):
    possible_sense_proto = [ChessSquareToProtobufPosition(square) for square in possible_sense]
    possible_moves_proto = [ChessMoveToProtobuf(move) for move in possible_moves]
    return agent_pb2.ChooseSenseRequest(
            possible_sense = possible_sense_proto,
            possible_moves = possible_moves_proto,
            seconds_left = seconds_left)

def MakeHandleSenseResultRequest(sense_result):
    result_list = [ChessSenseResultToProtobuf(result) for result in sense_result]
    return agent_pb2.HandleSenseResultRequest(
        result = result_list)


def MakeChooseMoveRequest(possible_moves, seconds_left):
    possible_moves = [ChessMoveToProtobuf(move) for move in possible_moves]
    return agent_pb2.ChooseMoveRequest(
            possible_moves = possible_moves,
            seconds_left = seconds_left)

def MakeHandleMoveResultRequest(requested_move, taken_move, reason, captured_square):
    if captured_square is None:
        return agent_pb2.HandleMoveResultRequest(
            requested_move = ChessMoveToProtobuf(requested_move),
            taken_move = ChessMoveToProtobuf(taken_move) if taken_move else None,
            reason = reason
        )
    return agent_pb2.HandleMoveResultRequest(
        requested_move = ChessMoveToProtobuf(requested_move),
        taken_move = ChessMoveToProtobuf(taken_move) if taken_move else None,
        reason = reason,
        captured_position = ChessSquareToProtobufPosition(captured_square)
    )

def MakeHandleGameEndRequest(winner_color, win_reason):
    return agent_pb2.HandleGameEndRequest(
        winner_color = ChessColorToProtobuf(winner_color),
        win_reason = win_reason)

class ServerAgent(Player):
    def __init__(self):
        self._channel = grpc.insecure_channel(SERVER_IP)
        self.stub = agent_pb2_grpc.RemoteAgentStub(self._channel)

    def __del__(self):
        self._channel.close()

    def handle_game_start(self, color, board):
        request = MakeHandleGameStartRequest(color, board)
        self.stub.HandleGameStart(request)

    def handle_opponent_move_result(self, captured_piece, captured_square):
        request = MakeHandleOpponentMoveRequest(captured_square if captured_piece else None)
        self.stub.HandleOpponentMove(request)

    def choose_sense(self, possible_sense, possible_moves, seconds_left):
        request = MakeChooseSenseRequest(possible_sense, possible_moves, seconds_left)
        response = self.stub.ChooseSense(request)
        return ProtobufPositionToChessSquare(response.sense_location)

    def handle_sense_result(self, sense_result):
        request = MakeHandleSenseResultRequest(sense_result)
        self.stub.HandleSenseResult(request)
        
    def choose_move(self, possible_moves, seconds_left):
        request = MakeChooseMoveRequest(possible_moves, seconds_left)
        response = self.stub.ChooseMove(request)
        return ProtobufMoveToChess(response.move)

    def handle_move_result(self, requested_move, taken_move, reason, captured_piece,
            captured_square):
        request = MakeHandleMoveResultRequest(requested_move, taken_move, reason,
                captured_square if captured_piece else None)
        self.stub.HandleMoveResult(request)

    def handle_game_end(self, winner_color, win_reason):
        request = MakeHandleGameEndRequest(winner_color, win_reason)
        self.stub.HandleGameEnd(request)
