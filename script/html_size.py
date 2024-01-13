import os

Import("env")

html_files = env.GetProjectOption("board_build.embed_txtfiles").split()
print( html_files )

for html in html_files:
    tag = "-D" + html.upper().replace("HTML/", "SIZE_").replace(".", "_")
    val = str(os.stat(html).st_size)
    env.Append(CPPDEFINES = (tag, val))
