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
    source = dir + "\\.pio\\build\\" + name + "\\firmware.bin"
    if name == "gravity-debug" :
        target = dir + "\\bin\\firmware-debug.bin"
    if name == "gravity-release" :
        target = dir + "\\bin\\firmware.bin"
    print( "Copy file : " + source + " -> " + target )
    shutil.copyfile( source, target )

    target = dir + "\\bin\\version.json"
    ver = get_build_flag_value("CFG_APPVER")
    print( "Creating version.json" )
    f = open( target, "w" )
    f.write( "{ \"project\":\"gravmon\", \"version\":" + ver + " }" ) 
    f.close()


print( "Adding custom build step: ")
#env.AddPreAction("buildprog", after_build)
env.AddPostAction("buildprog", after_build)
#env.AddPreAction("upload", after_build)   
#env.AddPostAction("upload", after_build)
