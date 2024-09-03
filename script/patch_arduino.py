Import("env")
import shutil

framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
name = env.get( "PIOENV" )#print(FRAMEWORK_DIR)
dir = env.GetLaunchDir()

if name == "gravity32s3-release" :
    source = dir + "/patch/arduino/variants/lolin_s3_mini/pins_arduino.h"
    target = framework_dir + "/variants/lolin_s3_mini/pins_arduino.h"
    print( "Copy file : " + source + " -> " + target )
    shutil.copyfile( source, target )
