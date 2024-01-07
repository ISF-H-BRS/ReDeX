#!/usr/bin/python

from time import time
from sys import *

if len(argv) != 2:
    print("Usage: %s outfile" % argv[0])
    exit(2)

outfile = open(argv[1], 'w')
outfile.write("#ifndef BUILD_TIMESTAMP_H\n")
outfile.write("#define BUILD_TIMESTAMP_H\n\n")
outfile.write("#define BUILD_TIMESTAMP %d\n\n" % int(time()))
outfile.write("#endif /* BUILD_TIMESTAMP_H */\n")
outfile.close()
