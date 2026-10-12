Import("env")
import os
import shutil

# Post build step for the cuckoo_32c3 target only (see extra_scripts in
# [env:cuckoo_32c3]). Copies the build output to bin/ under names that cannot be
# mistaken for the standard GravityMon builds. The standard script/copy_firmware.py
# and script/create_versionjson.py are not used for this target, so
# bin/version.json is not touched.

def after_build(source, target, env):
    print( "Executing custom step (cuckoo copy firmware)" )
    dir  = env.GetLaunchDir()
    name = env.get( "PIOENV" )

    for src, dst in [ ( "firmware.bin", "cuckoo_32c3.bin" ),
                      ( "partitions.bin", "cuckoo_32c3_partitions.bin" ) ]:
        source = os.path.join( dir, ".pio", "build", name, src )
        target = os.path.join( dir, "bin", dst )
        print( "Copy file : " + source + " -> " + target )
        shutil.copyfile( source, target )

print( "Adding custom build step (cuckoo copy firmware): ")
env.AddPostAction("buildprog", after_build)
