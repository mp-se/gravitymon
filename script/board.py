Import("env")

board = env.BoardConfig().get_brief_data()['id']
print("Detected board: " + board)

env.Append(
    CPPDEFINES=[(board.upper(), 1)],
)

env.Append(
    CPPDEFINES=[("BOARD", env.StringifyMacro(board.upper()))],
)

# print(env.get("CPPDEFINES"))
