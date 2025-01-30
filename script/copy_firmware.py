Import("env")
import shutil

def get_build_flag_value(flag_name):
    build_flags = env.ParseFlags(env['BUILD_FLAGS'])
    flags_with_value_list = [build_flag for build_flag in build_flags.get('CPPDEFINES') if type(build_flag) == list]
    defines = {k: v for (k, v) in flags_with_value_list}
    return defines.get(flag_name)

def after_build(source, target, env):
    print( "Executing custom step " )
    dir    = env.GetLaunchDir()
    name   = env.get( "PIOENV" )
    if name == "gravity-debug" :
        target = dir + "/bin/firmware-debug.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    if name == "gravity-release" :
        target = dir + "/bin/firmware.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    elif name == "gravity32-release" :
        target = dir + "/bin/firmware32.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partitions32.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    elif name == "gravity32c3-release" :
        target = dir + "/bin/firmware32c3.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partitions32c3.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    elif name == "gravity32s2-release" :
        target = dir + "/bin/firmware32s2.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partitions32s2.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    elif name == "gravity32s3-release" :
        target = dir + "/bin/firmware32s3.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partitions32s3.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    elif name == "gravity32lite-release" :
        target = dir + "/bin/firmware32lite.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partitions32lite.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    elif name == "pressure32s3-release" :
        target = dir + "/bin/firmware32s3.bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

        target = dir + "/bin/partitions32s3.bin"
        source = dir + "/.pio/build/" + name + "/partitions.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

    else:
        board = env.BoardConfig().get_brief_data()['id']
        print("Custom board detected: " + board)

        target = dir + "/bin/custom-" + board.lower() + ".bin"
        source = dir + "/.pio/build/" + name + "/firmware.bin"
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

print( "Adding custom build step (copy firmware): ")
env.AddPostAction("buildprog", after_build)
