import os

Import("env")

html_files = env.GetProjectOption("board_build.embed_txtfiles").split()
print( html_files )

#print(env["CPPDEFINES"])

for html in html_files:
    #print(html)
    tag = "-D" + html.upper().replace("HTML/", "SIZE_").replace(".", "_")
    val = str(os.stat(html).st_size)
    #print( tag, val)
    env.Append(CPPDEFINES = (tag, val))

#print(env["CPPDEFINES"])
