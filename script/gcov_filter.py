import re

Import("env")

def add_gcov_flags(env, node):
    gcov_files = env.GetProjectOption("custom_gcov_files").split()
    match = node.get_path().replace('\\', '/').split("/")[-1]

    if match not in gcov_files and len(gcov_files) > 0:
      match = None
    elif len(gcov_files) == 0:
      match = re.search("src/[a-z.]*cpp$", node.get_path().replace('\\', '/'))

    if match is not None:
      print("Adding gcov flags for:", node.get_path())

      return env.Object(
          node,
          CPPDEFINES=env["CPPDEFINES"],
          CCFLAGS=env["CCFLAGS"] + ["-fprofile-arcs", "-ftest-coverage", "-fprofile-abs-path" ]
      )

    return node

env.AddBuildMiddleware(add_gcov_flags)
