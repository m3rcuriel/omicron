#!/usr/bin/env python3

"""
File Name:      example_py_client.py
Authors:        Lee Mracek
Date:           April 10, 2019

Description:    Example server-side implementation of the gRPC ServerAgent in Python
"""

import random
import sys
import time

from concurrent import futures

import glog as log

import gflags as flags

import grpc

from google.protobuf import empty_pb2

import kstachowicz3_lmracek3_agent_pb2 as agent_pb2
import kstachowicz3_lmracek3_agent_pb2_grpc as agent_pb2_grpc

log.setLevel("INFO")
FLAGS = flags.FLAGS

def position_to_string(position):
    """
    Convert a protobuf Position into a string for debugging.
    """
    return "(file: " + str(position.file) + ", rank: " + str(position.rank) + ")"

def move_to_string(move):
    """
    Convert a protobuf Move into a string for debugging.
    """
    return "to: " + position_to_string(move.to_square) + "\n" + "from: " + \
            position_to_string(move.from_square)

def piece_to_string(piece):
    """
    Converts a protobuf Piece into a string for debugging.
    """
    piece_name = agent_pb2.PieceType.Name(piece.piece_type)
    if piece_name == "KNIGHT":
        piece_name = "n"
    if piece.color == agent_pb2.WHITE:
        return piece_name[0].upper()
    return piece_name[0].lower()


def sense_result_to_string(result):
    """
    Converts a single sense result into a string for debugging.
    """
    if result.HasField("piece"):
        return position_to_string(result.square) + ": " + piece_to_string(result.piece)
    return position_to_string(result.square) + ": " + "EMPTY"

class RandomAgent():
    def __init__(self):
        self.yourcolor = -1

    def handle_game_start(self, request):
        """
        Handle the start of the game.

        We store our color to use later for prettier logging.
        """
        log.info("HandleGameStart")
        log.debug("Your color: " + agent_pb2.Color.Name(request.color))
        self.yourcolor = request.color
        return empty_pb2.Empty()

    def handle_opponent_move(self, request):
        """
        Handle the move of the opponent.

        We optionally log what happened during the move but otherwise ignore it.
        """
        log.info("HandleOpponentMove")
        if request.HasField("captured_square"):
            log.debug("Opponent capture at square: " + position_to_string(request.captured_square))
        else:
            log.debug("No capture this round")
        return empty_pb2.Empty()

    def choose_sense(self, request):
        """
        Choose a position to sense around.

        Randomly select from a list and returns that in the reply. We output the chosen sense
        position and optoinally the full list of possible positions.
        """
        choose = random.choice(request.possible_sense)
        log.debug("Choose sense possible positions: " +
                  ", ".join([position_to_string(sense) for sense in request.possible_sense]))
        log.info("ChooseSense chose: " + position_to_string(choose))
        return agent_pb2.ChooseSenseReply(sense_location=choose)

    def handle_sense_result(self, request):
        """
        Handle the results of a sense move.

        We optionally print them out, but don't use them and return the empty message.
        """
        log.info("HandleSenseResult")
        log.debug("Sense results are: " +
                  ", ".join([sense_result_to_string(result) for result in request.result]))
        return empty_pb2.Empty()


    def choose_move(self, request):
        """
        Choose a move based on the set of possible moves.

        Randomly selects from the list and returns that reply. We output the chosen move,
        and optionally the full list of moves.
        """
        choice = random.choice(request.possible_moves)
        log.debug("Possible moves: \n" +
                  "\n".join([move_to_string(move) for move in request.possible_moves]))
        log.info("ChooseMove choose: " +
                 move_to_string(choice) + " with " + str(request.seconds_left) + " left")
        return agent_pb2.ChooseMoveReply(move=choice)

    def handle_move_result(self, request):
        """
        Handle the result of the move.

        If the move differs from the requested move, optionally log that.

        Return an empty message as the client doesn't need a response.
        """
        log.info("HandleMoveResult")
        log.debug("Requested Move: " + move_to_string(request.requested_move))
        if request.HasField("taken_move"):
            log.debug("Taken Move: " + move_to_string(request.taken_move))
        else:
            log.debug("Move not taken")
        log.debug("Reason: " + request.reason)
        if request.HasField("captured_position"):
            log.debug("Captured: " + position_to_string(request.captured_position))
        return empty_pb2.Empty()

    def handle_game_end(self, request):
        """
        Handle the end of the game by dumping out who won and why.

        We return an empty message since the client doesn't need to know anything.
        """
        if request.winner_color == self.yourcolor:
            log.info("Congratulations you won!")
        else:
            log.info("Better luck next time :(")
        log.info("Winning reason: " + request.win_reason)
        return empty_pb2.Empty()



class RandomAgentServicer(agent_pb2_grpc.RemoteAgentServicer):
    """
    Concrete implementation of the generated RemoteAgentServicer.

    The only fancy thing here (and really unnecessary for the purposes of the project) is that
    we store a dict of peers so that we can handle multiple games simultaneously. Since we only
    play one game at a time, this is not required for a successful implementation.
    """

    def __init__(self):
        self.peer_map = {}

    def HandleGameStart(self, request, context):
        log.info("Got game start from " + context.peer())
        self.peer_map[context.peer()] = RandomAgent()
        return self.peer_map[context.peer()].handle_game_start(request)

    def HandleOpponentMove(self, request, context):
        return self.peer_map[context.peer()].handle_opponent_move(request)

    def ChooseSense(self, request, context):
        return self.peer_map[context.peer()].choose_sense(request)

    def HandleSenseResult(self, request, context):
        return self.peer_map[context.peer()].handle_sense_result(request)

    def ChooseMove(self, request, context):
        return self.peer_map[context.peer()].choose_move(request)

    def HandleMoveResult(self, request, context):
        return self.peer_map[context.peer()].handle_move_result(request)

    def HandleGameEnd(self, request, context):
        res = self.peer_map[context.peer()].handle_game_end(request)
        del self.peer_map[context.peer()]
        return res

def main():
    """
    Main function to initialize logging and spin up the server.

    After the server is started we have to just sleep forever. There is no Wait() call like there
    is in gRPC libraries for other languages.
    """

    log.init()
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=5))
    agent_pb2_grpc.add_RemoteAgentServicer_to_server(RandomAgentServicer(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    try:
        while True:
            time.sleep(60 * 60 * 24)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == "__main__":
    FLAGS(sys.argv)
    main()
