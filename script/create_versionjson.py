Import("env", "projenv")
import shutil

def get_build_flag_value(flag_name):
    build_flags = env.ParseFlags(env['BUILD_FLAGS'])
    flags_with_value_list = [build_flag for build_flag in build_flags.get('CPPDEFINES') if type(build_flag) == list]
    defines = {k: v for (k, v) in flags_with_value_list}
    return defines.get(flag_name)

def after_build(source, target, env):
    print( "Executing custom step " )
    dir    = env.GetLaunchDir()
    #name   = env.get( "PIOENV" )

    # Copy file 1
    source = dir + "\\data\\index.min.htm"
    target = dir + "\\bin\\index.min.htm"
    print( "Copy file : " + source + " -> " + target )
    shutil.copyfile( source, target )

    # Copy file 2
    source = dir + "\\data\\device.min.htm"
    target = dir + "\\bin\\device.min.htm"
    print( "Copy file : " + source + " -> " + target )
    shutil.copyfile( source, target )

    # Copy file 3
    source = dir + "\\data\\config.min.htm"
    target = dir + "\\bin\\config.min.htm"
    print( "Copy file : " + source + " -> " + target )
    shutil.copyfile( source, target )

    # Copy file 4
    source = dir + "\\data\\about.min.htm"
    target = dir + "\\bin\\about.min.htm"
    print( "Copy file : " + source + " -> " + target )
    shutil.copyfile( source, target )

    target = dir + "\\bin\\version.json"
    ver = get_build_flag_value("CFG_APPVER")

    print( "Creating version.json" )
    f = open( target, "w" )
    f.write( "{ \"project\":\"gravmon\", \"version\":" + ver + ", " ) 
    f.write( " \"html\": [ \"index.min.htm\", \"device.min.htm\", \"config.min.htm\", \"about.min.htm\" ] }" )
    f.close()


print( "Adding custom build step: (create json)")
env.AddPreAction("buildprog", after_build)
