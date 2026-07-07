Import("env", "projenv")
import shutil
import json
import glob
import os
import gzip

def get_build_flag_value(flag_name):
    build_flags = env.ParseFlags(env['BUILD_FLAGS'])
    flags_with_value_list = [build_flag for build_flag in build_flags.get('CPPDEFINES') if type(build_flag) == list]
    defines = {k: v for (k, v) in flags_with_value_list}
    return defines.get(flag_name)

# Display name per locale code, matching ui/src/views/DeviceSettingsView.vue's
# localeOptions. "en" is excluded since it ships embedded in the firmware,
# not as a downloadable pack.
LANGUAGE_NAMES = {
    "sv": "Svenska",
    "de": "Deutsch",
    "fr": "Français",
    "es": "Español",
    "zh": "中文",
    "pl": "Polski",
    "no": "Norsk",
    "da": "Dansk",
    "nl": "Nederlands",
    "pt": "Português",
    "it": "Italiano",
}

def get_available_languages(dir):
    languages = []
    for path in sorted(glob.glob(dir + "/ui/src/locales/*.json")):
        code = os.path.splitext(os.path.basename(path))[0]
        if code not in LANGUAGE_NAMES:
            continue
        languages.append({
            "code": code,
            "name": LANGUAGE_NAMES[code],
            "file": "lang_" + code + ".json.gz",
        })
    return languages

# Gzips each available locale file into bin/lang_<code>.json.gz, matching the
# "file" name already recorded for it in version.json's "languages" list.
# These, together with version.json, are what get uploaded to
# https://www.gravitymon.com/firmware/ for the Device Settings language pack
# toggle list and manual Tools > Language Packs upload to fetch.
def build_language_packs(dir, languages):
    bin_dir = dir + "/bin"
    for entry in languages:
        src = dir + "/ui/src/locales/" + entry["code"] + ".json"
        dst = bin_dir + "/" + entry["file"]
        with open(src, "rb") as f_in:
            data = f_in.read()
        with gzip.open(dst, "wb") as f_out:
            f_out.write(data)
        print( "Built " + entry["file"] )

def after_build(source, target, env):
    print( "Executing custom step " )
    dir    = env.GetLaunchDir()
    #name   = env.get( "PIOENV" )

    target = dir + "/bin/version.json"
    ver = get_build_flag_value("CFG_APPVER")
    languages = get_available_languages(dir)

    print( "Building language packs" )
    build_language_packs(dir, languages)

    print( "Creating version.json" )
    languages_json = json.dumps(languages)
    f = open( target, "w" )
    f.write( "{ \"project\":\"gravmon\", \"version\":" + ver + ", " )
    f.write( " \"html\": [ ], \"languages\": " + languages_json + " }" )
    f.close()


print( "Adding custom build step (create json):")
env.AddPostAction("buildprog", after_build)
