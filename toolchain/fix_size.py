
import struct
import sys
from shutil import copyfile

filename = sys.argv[-1]
orig = open(filename, "rb")
contents = bytearray(orig.read())
orig.close()

# patch in the size as BE 32bit word
contents[4:8] = struct.pack(">I", len(contents))

new = open(filename, "wb")
new.write(contents)
new.close()
