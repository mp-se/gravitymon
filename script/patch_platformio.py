Import("env")
import shutil

try:
    framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
    dir = env.GetLaunchDir()

    platform_dir = env.PioPlatform().get_dir()

    source = dir+ "/patch/platforms/espressif32/"
    target = platform_dir + "/"
    print( "Copy directory : " + source + " -> " + target )
    shutil.copytree(source, target, dirs_exist_ok=True)

    source = dir + "/patch/packages/framework-arduinoespressif32/"
    target = framework_dir + "/"
    print( "Copy directory : " + source + " -> " + target )
    shutil.copytree(source, target, dirs_exist_ok=True)
except:
    pass

