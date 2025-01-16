Import("env")
import shutil

framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
name = env.get( "PIOENV" )#print(FRAMEWORK_DIR)
dir = env.GetLaunchDir()

if name == "gravity32c3-olimex-release" :
    source = dir + "/patch/olimex/pins_arduino.h"
    target = framework_dir + "/variants/esp_c3_m1_i_kit/pins_arduino.h"
#    print( "Copy file : " + source + " -> " + target )
#    shutil.copyfile( source, target )
