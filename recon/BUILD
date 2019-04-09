package(default_visibility = ["//visibility:public"])

py_library(
    name = "player",
    srcs = [
        "player.py",
    ],
)

py_library(
    name = "game",
    srcs = [ "game.py" ],
)

py_library(
    name = "random_agent",
    srcs = [ "random_agent.py" ],
)

py_binary(
    name = "play_game",
    srcs = [
        "play_game.py",
    ],
    deps = [
        ":player",
        ":game",
    ],
)

py_binary(
    name = "play_server_agent_against_random",
    main = "play_game.py",
    srcs = [
        "play_game.py",
    ],
    args = [
        "$(location :random_agent)",
        "$(location //server_agent:server_agent)",
    ],
    deps = [
        ":player",
        ":game",
        ":random_agent",
        "//server_agent:server_agent",
    ],
)
