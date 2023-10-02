import sys
import shutil
import os

#gcov_path = '~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-gcov'
#gcov_path = '~/.platformio/packages/toolchain-xtensa-esp32s2/bin/xtensa-esp32s2-elf-gcov'
gcov_path = 'c:/users/magnu/.platformio/packages/toolchain-xtensa-esp32s2/bin/xtensa-esp32s2-elf-gcov.exe'
gcov_args = "-jc"

args = len(sys.argv)
if args != 3:
  print("Usage: gcov_convert.py <input_file> <output_dir>")
  exit(0)

fname = sys.argv[1]
outpath = sys.argv[2]
print("Processing:", fname)

fin = open(fname, "r")
flines = fin.readlines()
fin.close()

out = bytearray()
outfiles = []
readBlock = False

for fline in flines:
  s = fline.strip()

  if s.startswith("00000000:"):
    readBlock = True

  # Parsing data block
  if readBlock and len(s)>8 and s[8] == ':':
    s = s[10:] # Get rid of the adress part
    byte = s.split(" ")

    for b in byte:
      #b = bytes(b, 'utf-8')
      out += bytes.fromhex(b)

  # Found filename so time to dump the data
  elif readBlock and s.endswith('.gcda') and not s.startswith('Emitting'):
    outname = s.replace('\\', '/').split('/')[-1] 
    print("Processed", len(out), "for", outname)

    outfiles.append(s)
    fout = open(outpath + "/" + outname, "wb")
    fout.write(out)
    fout.close()

    shutil.copyfile(s.replace('.gcda', '.gcno'), outpath + "/" + outname.replace('.gcda', '.gcno'))

    os.chdir(outpath)
    print("Changing working directory to:", os.getcwd())

    cmd = gcov_path + " " + gcov_args + " " + outname
    print("Running gcov:", cmd)
    os.system( cmd )

    # time to search for a new block
    readBlock = False
    out = bytearray()

print("Conversion completed.")