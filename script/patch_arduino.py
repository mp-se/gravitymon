Import("env")
import shutil
import pathlib

framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
name = env.get( "PIOENV" )#print(FRAMEWORK_DIR)
board = env.BoardConfig().get_brief_data()['id']
dir = env.GetLaunchDir()

source = dir + "/boards/"+ board +".pins.h"
pathlib.Path(framework_dir + "/variants/"+ board).mkdir(parents=True, exist_ok=True)
target = framework_dir + "/variants/"+ board +"/pins_arduino.h"
print( "Copy file : " + source + " -> " + target )
shutil.copyfile( source, target )
