"""
Fail the build if the firmware image does not fit in the app partition.

The size check that PlatformIO does is based on the sections in the ELF file
and reports a size that is lower than the real image, so an image that is too
large for the partition will still build and upload without any error. The
bootloader will then refuse to start it and the device ends up in a boot loop.
"""
Import("env")
import csv
import os


def app_partition_size(path):
    """Return the size of the first app partition in the partition table."""
    with open(path, newline="") as f:
        for row in csv.reader(f):
            row = [col.strip() for col in row]
            if len(row) < 5 or row[0].startswith("#"):
                continue
            if row[1] == "app":
                return int(row[4], 0)
    return None


def check_size(source, target, env):
    partitions = env.GetProjectOption("board_build.partitions", None)

    if not partitions:
        return  # ESP8266 or a board that use the default partition table

    path = os.path.join(env.subst("$PROJECT_DIR"), partitions)
    if not os.path.exists(path):
        print("Size check: Partition table %s not found, skipping" % partitions)
        return

    limit = app_partition_size(path)
    if not limit:
        print("Size check: No app partition in %s, skipping" % partitions)
        return

    size = os.path.getsize(str(target[0]))
    free = limit - size

    if free < 0:
        print(
            "Error: Firmware image is %d bytes but the app partition in %s is %d "
            "bytes, the image is %d bytes too large and will not boot."
            % (size, partitions, limit, -free)
        )
        # Remove the image so its not flashed by mistake and so it will be
        # created again on the next build, then fail this build step.
        os.remove(str(target[0]))
        return 1

    print(
        "Size check: Firmware image is %d bytes, %d bytes free in app partition"
        % (size, free)
    )


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", check_size)
