import grpc

import time

from concurrent import futures

import random
import sys

import agent_pb2
import agent_pb2_grpc

from google.protobuf import empty_pb2

import glog as log
import gflags as flags

import sys

FLAGS = flags.FLAGS

log.setLevel("INFO")

class RandomAgent():
    def __init__(self):
        pass

def PositionToString(position):
    return "(file: " + str(position.file) + ", rank: " + str(position.rank) + ")"

def MoveToString(move):
    return "to: " + PositionToString(move.to_square) + "\n" + "from: " + PositionToString(move.from_square)

def PieceToString(piece):
    piece_name = agent_pb2.PieceType.Name(piece.piece_type)
    if piece_name == "KNIGHT":
        piece_name = "n"
    if piece.color == agent_pb2.WHITE:
        return piece_name[0].upper()
    else:
        return piece_name[0].lower()


def SenseResultToString(result):
    if result.HasField("piece"):
        return PositionToString(result.square) + ": " + PieceToString(result.piece)
    else:
        return PositionToString(result.square) + ": " + "EMPTY"

class RandomAgent():
    def HandleGameStart(self, request):
        log.info("HandleGameStart")
        log.debug("Your color: " + agent_pb2.Color.Name(request.color))
        self.yourcolor = request.color
        return empty_pb2.Empty()

    def HandleOpponentMove(self, request):
        log.info("HandleOpponentMove")
        if request.HasField("captured_square"):
            log.debug("Opponent capture at square: " + PositionToString(request.captured_square))
        else:
            log.debug("No capture this round")
        return empty_pb2.Empty()

    def ChooseSense(self, request):
        choose = random.choice(request.possible_sense)
        log.debug("Choose sense possible positions: " + ", ".join([PositionToString(sense) for sense in request.possible_sense]))
        log.info("ChooseSense chose: " + PositionToString(choose))
        return agent_pb2.ChooseSenseReply(sense_location = choose)

    def HandleSenseResult(self, request):
        log.info("HandleSenseResult")
        log.debug("Sense results are: " + ", ".join([SenseResultToString(result) for result in request.result]))
        return empty_pb2.Empty()


    def ChooseMove(self, request):
        choice = random.choice(request.possible_moves)
        log.debug("Possible moves: \n" + "\n".join([MoveToString(move) for move in request.possible_moves]))
        log.info("ChooseMove choose: " + MoveToString(choice) + " with " + str(request.seconds_left) + " left")
        return agent_pb2.ChooseMoveReply(move = choice)

    def HandleMoveResult(self, request):
        log.info("HandleMoveResult")
        log.debug("Requested Move: " + MoveToString(request.requested_move))
        if request.HasField("taken_move"):
            log.debug("Taken Move: " + MoveToString(request.taken_move))
        else:
            log.debug("Move not taken")
        log.debug("Reason: " + request.reason)
        if request.HasField("captured_position"):
            log.debug("Captured: " + PositionToString(request.captured_position))
        return empty_pb2.Empty()

    def HandleGameEnd(self, request):
        if request.winner_color == self.yourcolor:
            log.info("Congratulations you won!")
        else:
            log.info("Better luck next time :(")
        log.info("Winning reason: " + request.win_reason)
        return empty_pb2.Empty()



class RandomAgentServicer(agent_pb2_grpc.RemoteAgentServicer):
    def __init__(self):
        self.peer_map = {}

    def HandleGameStart(self, request, context):
        log.info("Got game start from " + context.peer())
        self.peer_map[context.peer()] = RandomAgent()
        return self.peer_map[context.peer()].HandleGameStart(request)

    def HandleOpponentMove(self, request, context):
        return self.peer_map[context.peer()].HandleOpponentMove(request)

    def ChooseSense(self, request, context):
        return self.peer_map[context.peer()].ChooseSense(request)

    def HandleSenseResult(self, request, context):
        return self.peer_map[context.peer()].HandleSenseResult(request)

    def ChooseMove(self, request, context):
        return self.peer_map[context.peer()].ChooseMove(request)

    def HandleMoveResult(self, request, context):
        return self.peer_map[context.peer()].HandleMoveResult(request)

    def HandleGameEnd(self, request, context):
        res = self.peer_map[context.peer()].HandleGameEnd(request)
        del self.peer_map[context.peer()]
        return res



if __name__ == "__main__":
    FLAGS(sys.argv)
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
