import os

filename = "/usr/share/cs644/bigfile.txt"

fd = os.open(filename, os.O_WRONLY)
print(fd)
s = os.read(fd, 4096)
print(s)
os.close(fd)
