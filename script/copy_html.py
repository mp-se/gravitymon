Import("env")
import shutil

print( "Executing custom step " )
dir    = env.GetLaunchDir()
source = dir + "/html/"
target = dir + "/data/"
print( "Copy file : " + source + " -> " + target )

file = "about.min.htm"
print( "Copy file: " + source + file + "->" + target + file)
shutil.copyfile( source + file, target + file )
file = "calibration.min.htm"
print( "Copy file: " + source + file + "->" + target + file)
shutil.copyfile( source + file, target + file )
file = "config.min.htm"
print( "Copy file: " + source + file + "->" + target + file)
shutil.copyfile( source + file, target + file )
file = "device.min.htm"
print( "Copy file: " + source + file + "->" + target + file)
shutil.copyfile( source + file, target + file )
file = "index.min.htm"
print( "Copy file: " + source + file + "->" + target + file)
shutil.copyfile( source + file, target + file )
file = "upload.min.htm"
print( "Copy file: " + source + file + "->" + target + file)
shutil.copyfile( source + file, target + file )

#print( "Adding custom build step (copy ./heml/*.min.html to ./data/): ")
#print(env.Dump())
#env.AddPreAction("buildprog", before_build)
