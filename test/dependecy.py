import requests
import configparser
import json

debug = False
useFiles = False

githubUrl = "https://api.github.com/repos/"

def get_github_latest_tag(repo):
    if repo == "":
        return ""

    if debug: print("Debug:", githubUrl + repo + "/tags")
    response = requests.get(githubUrl + repo + "/tags")
    if debug: print("Debug:", "Return code:", response.status_code)

    if response.status_code == 403:
       print( "Error:", "Probably rate limiting has kicked in.....") 

    if response.status_code == 200: 
      json = response.json()
      if debug: write_json("debug_tag.json", json)
    
      if len(json) > 0:
        if debug: print("Debug:", repo, json[0]["name"])
        return json[0]["name"]

    # print("Error:", "No releases found for:", repo)
    return ""

def get_github_parent(repo):
    if repo == "":
        return ""

    if debug: print("Debug:", githubUrl + repo)
    response = requests.get(githubUrl + repo)
    if debug: print("Debug:", "Return code:", response.status_code)

    if response.status_code == 200:
      json = response.json()
      if "parent" in json:
        if debug: print("Debug:", repo, json["parent"]["full_name"])
        if debug: write_json("debug_parent.json", json)
        return json["parent"]["full_name"]

    return ""
      
def get_github_repo_version(repoList):
  deps = []

  for repo in repoList:    
    tag = get_github_latest_tag(repo)
    parent = get_github_parent(repo)
    ptag = get_github_latest_tag(parent)
    dep = { "repo": repo, "tag": tag, "parent": parent, "parentTag": ptag }
    deps.append(dep)

  return deps

def resolve_platformio_key(config, key):
  key = key.removeprefix("${")
  key = key.removesuffix("}")
  libs = config.get(key.split(".")[0], key.split(".")[1])
  return libs.split()

def find_platformio_libs():
  config = configparser.ConfigParser()
  config.read('platformio.ini')

  projectRepos = []

  for section in config.sections():
    if section.startswith("env:"):
      libs = config.get(section, "lib_deps" )
      for lib in libs.split():
        if lib.startswith("${") and lib.endswith("}"):
          projectRepos = projectRepos + resolve_platformio_key(config, lib)
        else:
          projectRepos.append(lib)

  # Remove duplicated entries
  projectRepos = list(set(projectRepos))

  # list of the following objects { "repo": "", "tag": "" }
  dependecies = []

  for dep in projectRepos:
    s = dep.removeprefix("https://github.com/").split("#")
    if len(s) == 2:
      e = { "repo": s[0], "tag": s[1] }
      dependecies.append(e)
    else:
      e = { "repo": s[0], "tag": "" }
      dependecies.append(e)

  # Entires have format; repository # tag 
  return dependecies

def write_json(file, data):
  f = open(file, "w")
  f.write( json.dumps(data, indent=2) )
  f.close()

def read_json(file):
  f = open(file, "r")
  data = json.loads( "".join(f.readlines()) )
  f.close()
  return data

if __name__ == '__main__':
  print("Dependency checker")

  print("Reading platformio configuration")
  if useFiles: 
    depLibs = read_json("project_deps.json")
  else:
    depLibs = find_platformio_libs()
    write_json("project_deps.json", depLibs)

  print("Fetching data from github on latest tags")
  if useFiles: 
    repos = read_json("repos.json")
  else:
    repoList = []
    for d in depLibs:
      repoList.append(d["repo"])

    repos = get_github_repo_version(repoList)
    write_json("repos.json", repos)

  for i in depLibs:
    for j in repos:
      if i["repo"] == j["repo"]:
        if i["tag"] != j["tag"] or j["tag"] != j["parentTag"]:
          print("Check dependency for:", i["repo"], "platformio:", i["tag"], "git:", j["tag"]," => parent:", j["parentTag"] )

  print("Done...")
