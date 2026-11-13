Import("env")
import os
import shutil

# Suffix used in the output file names: firmware<suffix>.bin and partitions<suffix>.bin
# The ESP8266 build has no suffix and no partitions file.
SUFFIXES = {
    "gravity-8266": "",
    "gravity-32c3_mini": "32c3",
    "gravity-32c3_pico": "32c3pico",
    "gravity-32c3_zero": "32c3zero",
    "gravity-32c3_supermini": "32c3supermini",
    "gravity-32c3_cuckoo": "32c3cuckoo",
    "gravity-32c3_cuckoo_v2": "32c3cuckoo2",
    "gravity-32s2_mini": "32s2",
    "gravity-32s3_mini": "32s3",
    "gravity-32s3_zero": "32s3zero",
}

def copy(source, target):
    print("Copy file : " + source + " -> " + target)
    shutil.copyfile(source, target)

def after_build(source, target, env):
    print("Executing custom step ")
    dir = env.GetLaunchDir()
    name = env.get("PIOENV")
    build = dir + "/.pio/build/" + name

    if name == "gravity-unit32":
        print("Skipping copy of unit test build")
        return

    # Hardware test firmware (gravity-hw-<board>) is stored separately in bin/hw-test
    # as hardware<suffix>.bin so it is not flashed by mistake. Suffix is the same as the standard build.
    if name.startswith("gravity-hw-"):
        outdir = dir + "/bin/hw-test"
        os.makedirs(outdir, exist_ok=True)
        prefix = "hardware"
        suffix = SUFFIXES.get("gravity-" + name[len("gravity-hw-"):])
    else:
        outdir = dir + "/bin"
        prefix = "firmware"
        suffix = SUFFIXES.get(name)

    if suffix is not None:
        copy(build + "/firmware.bin", outdir + "/" + prefix + suffix + ".bin")
        if os.path.exists(build + "/partitions.bin"):
            copy(build + "/partitions.bin", outdir + "/partitions" + suffix + ".bin")
    else:
        # Custom boards
        board = env.BoardConfig().get_brief_data()['id']
        print("Custom board detected: " + board)
        copy(build + "/firmware.bin", dir + "/bin/custom-" + board.lower() + ".bin")

print("Adding custom build step (copy firmware): ")
env.AddPostAction("buildprog", after_build)
