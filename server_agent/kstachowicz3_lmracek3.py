#!/usr/bin/env python3

"""
File Name:      remote_agent.py
Authors:        Lee Mracek
Date:           April 9, 2019

Description:    Python file for an agent running on a remote server via gRPC
"""

import os

import chess
import grpc
import random

from player import Player

import kstachowicz3_lmracek3_agent_pb2 as agent_pb2
import kstachowicz3_lmracek3_agent_pb2_grpc as agent_pb2_grpc

SERVER_IP = "omicron.leemracek.com:50051"
SERVER_IP = os.getenv("SERVER_IP", SERVER_IP)
"""
The IP for the ServerAgent to connect to. Note that this can be edited above in this file
or overridden by the environment variable SERVER_IP.
"""

def chess_color_to_protobuf(color):
    """
    Utility function to convert from the Protobuf serialized color to the python-chess color
    representation.

    :param color: chess.BLACK or chess.WHITE
    :return: agent_pb2.BLACK or agent_pb2.WHITE
    """
    if color == chess.WHITE:
        return agent_pb2.WHITE
    if color == chess.BLACK:
        return agent_pb2.BLACK
    raise ValueError("Please don't feed me invalid colors")

def chess_square_to_protobuf_position(square):
    """
    Utility function to convert from the python-chess square to a Protobuf position.

    :param square: chess.Square to be converted
    :return: agent_pb2.Position equivalent to square
    """
    return agent_pb2.Position(
        rank=chess.square_rank(square),
        file=chess.square_file(square))

def protobuf_position_to_chess_square(position):
    """
    Utility function to convert from the Protobuf position to a python-chess square.

    :param position: agent_pb2.Position to convert
    :return: chess.Square equivalent to position
    """
    return chess.square(position.file, position.rank)

def protobuf_move_to_chess(move):
    """
    Utility function to convert from a Protobuf move to a chess.Move.

    :param move: agent_pb2.Move to convert
    :return: chess.Move equivalent to move
    """
    to_position = protobuf_position_to_chess_square(move.to_square)
    from_position = protobuf_position_to_chess_square(move.from_square)
    promotion = None
    if move.has_promotion:
        promotion = move.promotion
    return chess.Move(from_position, to_position, promotion)

def chess_piece_to_protobuf(piece):
    """
    Utility function to convert chess.Piece into a Protobuf piece.

    :param piece: chess.Piece to convert
    :return: agent_pb2.Piece equivalent to piece
    """
    piece_type = chess_piece_type_to_protobuf(piece.piece_type)
    piece = agent_pb2.Piece(piece_type=piece_type,
                           color=chess_color_to_protobuf(piece.color))
    return piece

def chess_piece_type_to_protobuf(piece_type):
    """
    Utility function to convert the chess.PieceType enum to the Protobuf PieceType enum.

    :param piece_type: chess.PieceType to convert
    :return: agent_pb2.PieceType equivalent to piece_type
    """
    if piece_type == chess.PAWN:
        return agent_pb2.PAWN
    if piece_type == chess.KNIGHT:
        return agent_pb2.KNIGHT
    if piece_type == chess.BISHOP:
        return agent_pb2.BISHOP
    if piece_type == chess.ROOK:
        return agent_pb2.ROOK
    if piece_type == chess.QUEEN:
        return agent_pb2.QUEEN
    if piece_type == chess.KING:
        return agent_pb2.KING
    raise ValueError("Please pass a valid chess piece")


def chess_move_to_protobuf(move):
    """
    Utility function to convert the chess move into a Protobuf move.

    :param move: chess.Move to convert
    :return: agent_pb2.Move equilvalent to move
    """
    to_position = chess_square_to_protobuf_position(move.to_square)
    from_position = chess_square_to_protobuf_position(move.from_square)
    if move.promotion is not None:
        promotion_piece = chess_piece_type_to_protobuf(move.promotion)
        return agent_pb2.Move(
            to_square=to_position,
            from_square=from_position,
            has_promotion=True,
            promotion=promotion_piece)
    return agent_pb2.Move(
        to_square=to_position,
        from_square=from_position,
        has_promotion=False)


def chess_sense_result_to_protobuf(result):
    """
    Utility function to serialize a sense result into Protobuf.

    :param result: tuple of chess.Square and chess.Piece representing a sense result
    :return: sense result converted into a Protobuf message
    """
    if result[1] is None:
        return agent_pb2.SenseResult(
            square=chess_square_to_protobuf_position(result[0]))
    return agent_pb2.SenseResult(
        square=chess_square_to_protobuf_position(result[0]),
        piece=chess_piece_to_protobuf(result[1]))


#TODO does it make any sense to include the initial board state here
def make_handle_game_start_request(color, board):
    """
    Construct a HandleGameStartRequest using the parameters passed to handle_game_start.

    :param color: chess.BLACK or chess.WHITE -- your color assignment for the game
    :param board: (Unused) -- the initial board state
    :return: A gRPC HandleGameStartRequest to pass into the stub
    """
    del board

    proto_color = chess_color_to_protobuf(color)

    return agent_pb2.HandleGameStartRequest(
        color=proto_color)

def make_handle_opponent_move_request(captured_square):
    """
    Construct a HandleOpponentMoveRequest using the parameters passed to
    handle_opponent_move_request.

    :param captured_square: chess.Square if a piece was captured, else None
    :return: A gRPC HandleOpponentMoveRequest
    """
    if captured_square is None:
        return agent_pb2.HandleOpponentMoveRequest()
    captured_square_proto = chess_square_to_protobuf_position(captured_square)
    return agent_pb2.HandleOpponentMoveRequest(captured_square=captured_square_proto)

def make_choose_sense_request(possible_sense, possible_moves, seconds_left):
    """
    Construct a ChooseSenseRequest using the parameters passed into choose_sense.

    :param possible_sense: List(chess.Squares), a list of squares to sense around
    :param possible_moves: List(chess.Moves), a list of possible moves from the game manager
    :param seconds_left: float, seconds left in the game

    :return: A gRPC ChooseSenseRequest to pass into the stub
    """
    possible_sense_proto = [chess_square_to_protobuf_position(square) for square in possible_sense]
    possible_moves_proto = [chess_move_to_protobuf(move) for move in possible_moves]
    return agent_pb2.ChooseSenseRequest(
        possible_sense=possible_sense_proto,
        possible_moves=possible_moves_proto,
        seconds_left=seconds_left)

def make_handle_sense_result_request(sense_result):
    """
    Construct a HandleSenseResultRequest using the parameters to handle_sense_result.

    :param sense_result: A list of tuples as in the docstring for handle_sense_result
    :return: A gRPC HandleSenseResultRequest to pass into the stub
    """
    result_list = [chess_sense_result_to_protobuf(result) for result in sense_result]
    return agent_pb2.HandleSenseResultRequest(
        result=result_list)


def make_choose_move_request(possible_moves, seconds_left):
    """
    Construct a ChooseMoveResult using the parameters to choose_move.

    :param possible_moves: List(chess.Square) the possible moves based on the board state

    :return: A gRPC ChooseMoveRequest to pass into the stub
    """
    possible_moves = [chess_move_to_protobuf(move) for move in possible_moves]
    return agent_pb2.ChooseMoveRequest(
        possible_moves=possible_moves,
        seconds_left=seconds_left)

def make_handle_move_result_request(requested_move, taken_move, reason, captured_square):
    """
    Construct a HandleMoveResultRequest using the parameters to handle_move_result.

    :params requested_move: The move that was initially requested
    :params taken_move: The move that was actually executed
    :params reason: The reason the move was not executed (if it wasn't)
    :params captured_square: chess.Square if a piece was captured, else None
    :return: The HandleMoveResultRequest to pass into the stub
    """
    if captured_square is None:
        return agent_pb2.HandleMoveResultRequest(
            requested_move=chess_move_to_protobuf(requested_move),
            taken_move=chess_move_to_protobuf(taken_move) if taken_move else None,
            reason=reason
        )
    return agent_pb2.HandleMoveResultRequest(
        requested_move=chess_move_to_protobuf(requested_move),
        taken_move=chess_move_to_protobuf(taken_move) if taken_move else None,
        reason=reason,
        captured_position=chess_square_to_protobuf_position(captured_square)
    )

def make_handle_game_end_request(winner_color, win_reason):
    """
    Construct a HandleGameEndRequest using the parameters to handle_game_end.

    :params winner_color: chess.BLACK or chess.WHITE, whichever won
    :params win_reason: The string reason why the game is over

    :return: The HandleGameEndRequest to pass into the stub
    """
    return agent_pb2.HandleGameEndRequest(
        winner_color=chess_color_to_protobuf(winner_color),
        win_reason=win_reason)

class ServerAgent(Player):
    """
    Agent conforming to the design of MyAgent that sends requests to gRPC to processing.

    This class is intended to serve as a drop-in replacement for the agent which runs directly
    on the laptop. Each call generates a gRPC request which is then passed along into the stub.

    No one should have to edit this class in order to get the gRPC setup working. All that must
    be done is setting the SERVER_IP via environment variable or at this top of this file and
    then ensuring that at that IP is the gRPC server listening to requests.

    The only state tracked in this class is the stub to communicate with the remote server.

    For more documentation for class methods look at the reference MyAgent implementation
    """
    def __init__(self):
        """
        Handles setting up the channel and stub to communicate with the server.
        """
        try:
            self._channel = grpc.insecure_channel(SERVER_IP)
            self.stub = agent_pb2_grpc.RemoteAgentStub(self._channel)
        except Exception as e:
            print(e)

    def __del__(self):
        """
        Closes the channel when the class is destroyed.
        """
        self._channel.close()

    def handle_game_start(self, color, board):
        """
        Handles the start of the game by calling out to the stub.
        """
        try:
            request = make_handle_game_start_request(color, board)
            self.stub.HandleGameStart(request)
        except Exception as e:
            print(e)

    def handle_opponent_move_result(self, captured_piece, captured_square):
        """
        Handles the result of the opponent's move.

        Transforms the captured_piece bool and value into just an optional value in order to build
        the request.
        """
        try:
            request = make_handle_opponent_move_request(captured_square if captured_piece else None)
            self.stub.HandleOpponentMove(request)
        except Exception as e:
            print(e)

    def choose_sense(self, possible_sense, possible_moves, seconds_left):
        """
        Chooses a position on the board to sense.

        This transforms the result of the gRPC call back into a chess.Square.
        """
        try:
            request = make_choose_sense_request(possible_sense, possible_moves, seconds_left)
            response = self.stub.ChooseSense(request)
            if response.HasField("sense_location"):
                return protobuf_position_to_chess_square(response.sense_location)
        except Exception as e:
            print(e)
            return random.choice(possible_sense)

    def handle_sense_result(self, sense_result):
        """
        Handles the result of the sensing move.
        """
        try:
            request = make_handle_sense_result_request(sense_result)
            self.stub.HandleSenseResult(request)
        except Exception as e:
            print(e)

    def choose_move(self, possible_moves, seconds_left):
        """
        Chooses a move based on the list of possible moves.

        This transforms the result of the gRPC call back into a chess.Move.
        """
        try:
            request = make_choose_move_request(possible_moves, seconds_left)
            response = self.stub.ChooseMove(request)
            if response.HasField("move"):
                return protobuf_move_to_chess(response.move)
        except Exception as e:
            print(e)
            return random.choice(possible_moves)

    def handle_move_result(self, requested_move, taken_move, reason, captured_piece,
                           captured_square):
        """
        Handle the result of the move returned by the game manager.

        Transforms the captured_piece bool and value into an optional value to build the request.
        """
        try:
            request = make_handle_move_result_request(requested_move, taken_move, reason,
                                                      captured_square if captured_piece else None)
            self.stub.HandleMoveResult(request)
        except Exception as e:
            print(e)

    def handle_game_end(self, winner_color, win_reason):
        """
        Handle the end of the game by calling out to the stub. This should clean up any state left
        on the server-side.
        """
        try:
            request = make_handle_game_end_request(winner_color, win_reason)
            self.stub.HandleGameEnd(request)
        except Exception as e:
            print(e)
